#include "sql/handler.h"
#include "thr_lock.h"
#include "sql/table.h"
#include <vector>
#include <string>

// Each Table Object has a name, rows and table_lock.
struct MememTable {
    std::vector<std::vector<uchar>> rows;
    std::string name;
    THR_LOCK lock; 
};

class ha_memem : public handler {
private:
    THR_LOCK_DATA lock;
    MememTable* mem_table;
    uint current_position;

public:
    ha_memem(handlerton* hton, TABLE_SHARE* table_arg);
    ~ha_memem() override;

    const char* table_type() const override;
    ulonglong table_flags() const override;
    ulong index_flags(uint inx, uint part, bool all_parts) const override;

    int create(const char* name, TABLE* form, HA_CREATE_INFO*,
               dd::Table* table_def) override;
    int open(const char* name, int mode, uint test_if_locked,
             const dd::Table* table_def) override;
    int close() override;

    int write_row(uchar* buf) override;
    int rnd_init(bool scan) override;
    int rnd_next(uchar* buf) override;
    int rnd_pos(uchar* buf, uchar* pos) override;
    void position(const uchar*) override;
    int info(uint) override;

    int external_lock(THD*, int) override;
    THR_LOCK_DATA** store_lock(THD*, THR_LOCK_DATA** to,
                               enum thr_lock_type lock_type) override;
};
