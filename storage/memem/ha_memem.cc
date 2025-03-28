#include "mysql/plugin.h"
#include "sql/sql_class.h"
#include "storage/memem/ha_memem.h"

//------------------------------------------------------
// Flags
//------------------------------------------------------

/**
 * @brief Get the storage engine name
 *
 * Returns the name used to identify this storage engine.
 * This name is used in SHOW TABLE STATUS and SHOW CREATE TABLE.
 *
 * @return const char* Storage engine name
 */
const char* ha_memem::table_type() const { return "MEMEM"; }

/**
 * @brief Get table flags that specify handler capabilities
 *
 * Returns a bitmap of flags that tells MySQL about the capabilities
 * of this storage engine. Current flags:
 *  - HA_NO_TRANSACTIONS: Engine doesn't support transactions
 *  - HA_BINLOG_ROW_CAPABLE: Can handle row-based binary logging
 *
 * @return ulonglong Bitmap of handler flags
 */
ulonglong ha_memem::table_flags() const {
    return HA_NO_TRANSACTIONS | HA_BINLOG_ROW_CAPABLE;
}

/**
 * @brief Get index capabilities
 *
 * Returns flags that indicate what kind of indexes this storage engine
 * supports. Returns 0 as this basic implementation doesn't support indexes.
 */
uLong ha_memem::index_flags(uint, uint, bool) const { return 0; }

//------------------------------------------------------
// Core
//------------------------------------------------------

//
// Called when: CREATE TABLE is executed
// Purpose: Initialize the table structure/files
//
int ha_memem::create(const char* name,
                     TABLE* form [[maybe_unused]],
                     HA_CREATE_INFO* create_info [[maybe_unused]],
                     dd::Table* table_def [[maybe_unused]]) {
    mem_table = new MememTable();
    mem_table->name = name;
    thr_lock_init(&mem_table->lock); // Initialize lock
    thr_lock_data_init(&mem_table->lock, &lock, NULL); // Initialize lock data
    return 0;
}

//
// Called when: Table is opened for operations
// Purpose: Open existing table for read/write
//
int ha_memem::open(const char* name,
                   int mode [[maybe_unused]],
                   uint test_if_locked [[maybe_unused]],
                   const dd::Table* table_def [[maybe_unused]]) {
    mem_table = new MememTable();
    mem_table->name = name;
    thr_lock_init(&mem_table->lock); // Initialize lock
    thr_lock_data_init(&mem_table->lock, &lock, NULL); // Initialize lock data
    return 0;
}

int ha_memem::close() {
    if (mem_table) {
        thr_lock_delete(&mem_table->lock); // Clean up lock
        delete mem_table;
        mem_table = nullptr;
    }
    return 0;
}


int ha_memem::write_row(uchar* buf) {
    size_t row_length = table->s->stored_rec_length;
    mem_table->rows.emplace_back(buf, buf + row_length);
    return 0;
}

/**
 * @brief Initialize table scanning
 *
 * Called before starting a table scan. Initializes position
 * for sequential reading of rows.
 *
 * @param scan True if this is a full table scan
 * @return int 0 for success, non-zero for failure
 */
int ha_memem::rnd_init(bool scan [[maybe_unused]]) {
    current_position = 0;
    return 0;
}

/**
 * @brief Read the next row in a table scan
 *
 * Reads the next row in a sequential scan and places it in
 * the provided buffer.
 *
 * @param buf Buffer to store the row
 * @return int 0 for success, HA_ERR_END_OF_FILE for end of table
 */
int ha_memem::rnd_next(uchar* buf) {
    if (current_position >= mem_table->rows.size()) {
        return HA_ERR_END_OF_FILE;
    }

    memcpy(buf, mem_table->rows[current_position].data(),
           mem_table->rows[current_position].size());
    current_position++;
    return 0;
}

/**
 * @brief Store position for later retrieval
 *
 * Stores current position for later retrieval by rnd_pos().
 * Used for ORDER BY and GROUP BY operations.
 *
 * @param record Currently unused
 */
void ha_memem::position(const uchar*) {
    size_t* position = reinterpret_cast<size_t*>(ref);
    *position = current_position - 1;
}

/**
 * @brief Read a row using position
 *
 * Reads a row from a given position. The position information
 * comes from an earlier call to position().
 *
 * @param buf Buffer to store the row
 * @param pos Position information
 * @return int 0 for success, HA_ERR_END_OF_FILE for invalid position
 */
int ha_memem::rnd_pos(uchar* buf, uchar* pos) {
    size_t position = *reinterpret_cast<size_t*>(pos);
    if (position >= mem_table->rows.size()) {
        return HA_ERR_END_OF_FILE;
    }

    memcpy(buf, mem_table->rows[position].data(),
           mem_table->rows[position].size());
    return 0;
}


/**
 * @brief Information about the table
 *
 * Called to get information about the table. Currently a no-op
 * in this basic implementation.
 *
 * @param flag Type of information requested
 * @return int 0 for success
 */
int ha_memem::info(uint) { return 0; }

//------------------------------------------------------
// Locks
//------------------------------------------------------

/**
 * @brief External table lock handler
 *
 * This function is called by MySQL at the beginning and end of every statement that
 * references this table. It manages external/file-level locks for the table.
 *
 * Called twice for each statement:
 *  1. At start with lock_type = F_RDLCK (read) or F_WRLCK (write)
 *  2. At end with lock_type = F_UNLCK
 *
 * For transactional tables, this should be a no-op as transactions
 * handle the locking automatically.
 *
 * @param thd Thread handler (unused in basic implementation)
 * @param lock_type Type of lock (F_RDLCK, F_WRLCK, F_UNLCK)
 * @return 0 for success, non-zero for failure
 */
int ha_memem::external_lock(THD* thd [[maybe_unused]], int lock_type [[maybe_unused]]) {
    DBUG_TRACE;
    return 0;
}

/**
 * @brief Store lock request in lock structure
 *
 * This method is called by MySQL to build a list of table-level locks needed
 * for a particular statement. It's called multiple times to build the complete
 * lock list before actually trying to acquire the locks.
 *
 * Lock upgrade protocol:
 *  - If lock_type is TL_IGNORE, keep existing lock type
 *  - If current lock is TL_UNLOCK, set to requested lock_type
 *  - Otherwise, keep existing lock type (don't downgrade)
 *
 * Common lock types:
 *  - TL_READ: Normal read lock
 *  - TL_READ_WITH_SHARED_LOCKS: READ with LOCK IN SHARE MODE
 *  - TL_WRITE: Normal write lock
 *  - TL_WRITE_ALLOW_WRITE: INSERT operations
 *
 * @param thd Thread handler
 * @param to Pointer to an array of lock requests
 * @param lock_type Type of lock requested
 * @return Position after the current lock in the lock array
 */
THR_LOCK_DATA** ha_memem::store_lock(THD* thd [[maybe_unused]],
                                     THR_LOCK_DATA** to,
                                     enum thr_lock_type lock_type) {
    // Only change lock type if:
    // 1. Lock type is not TL_IGNORE
    // 2. Current lock type is TL_UNLOCK (no lock)
    if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK) {
        lock.type = lock_type;
    }

    // Add this lock to the table's lock list
    *to++ = &lock;

    // Return position after our lock
    return to;
}

//--------------------------------------------------
// Plugin-specific declarations and definitions
//--------------------------------------------------

/**
 * @brief Create handler instance for the storage engine
 *
 * Called by MySQL to create a new handler instance when a table is opened.
 * Uses placement new to create the handler in the provided memory root.
 *
 * @param hton Handlerton for this storage engine
 * @param table TABLE_SHARE containing table definition
 * @param partitioned Whether table is partitioned (unused in basic implementation)
 * @param mem_root Memory root to allocate handler from
 * @return handler* Pointer to newly created handler instance
 */
static handler* memem_create_handler(handlerton* hton,
                                     TABLE_SHARE* table,
                                     bool,
                                     MEM_ROOT* mem_root) {
    return new (mem_root) ha_memem(hton, table);
}

ha_memem::ha_memem(handlerton* hton, TABLE_SHARE* table_arg)
    : handler(hton, table_arg), mem_table(nullptr), current_position(0) {}

ha_memem::~ha_memem() = default;

/** Global handlerton instance for the MEMEM storage engine */
handlerton *memem_hton;

/**
 * @brief Initialize the storage engine plugin
 *
 * Called when the storage engine is loaded. Initializes the handlerton
 * with the necessary function pointers and capabilities.
 */
static int memem_init_func(void* p) {
    memem_hton = (handlerton *)p;
    memem_hton->state = SHOW_OPTION_YES;             // Mark plugin as active
    memem_hton->create = memem_create_handler;       // Set handler creation function
    memem_hton->flags = HTON_CAN_RECREATE;           // Set capabilities
    return 0;
}

static int memem_deinit_func(void* p [[maybe_unused]]) {
    return 0;
}

/**
 * @brief Storage engine descriptor
 *
 * Contains version information for the storage engine interface.
 * Used by MySQL to verify compatibility.
 */
static struct st_mysql_storage_engine memem_storage_engine = {
    MYSQL_HANDLERTON_INTERFACE_VERSION
};

/**
 * @brief Plugin declaration structure
 *
 * Declares the storage engine plugin to MySQL, including:
 *  - Plugin type (storage engine)
 *  - Name and author information
 *  - License information
 *  - Initialize and cleanup functions
 *  - Version and status information
 */
mysql_declare_plugin(memem) {
    MYSQL_STORAGE_ENGINE_PLUGIN,
    &memem_storage_engine,       // Plugin descriptor
    "MEMEM",                     // Plugin name
    PLUGIN_AUTHOR_ORACLE,       // Author
    "MEMEM storage engine",     // Description
    PLUGIN_LICENSE_GPL,         // License
    memem_init_func,            // Initialization function
    nullptr,                    // Check uninstall function
    memem_deinit_func,          // Cleanup function
    0x0001,                     // Version number (0.1)
    nullptr,                    // Status variables
    nullptr,                    // System variables
    nullptr,                    // Config options
    0,                          // Flags
}
mysql_declare_plugin_end;