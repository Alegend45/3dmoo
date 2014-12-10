void stall_cpu(void *instance);
void unstall_cpu(void *instance);
u32 read_cpu_reg(void *instance, u32 reg_num, u32 handle);
void set_cpu_reg(void *instance, u32 reg_num, u32 value, u32 handle);
void install_post_exec_fn(void *instance, void(*ex_fn)(void *, u32 adr, int thumb), void *fn_data);
void remove_post_exec_fn(void *instance);
u16 gdb_prefetch16(void *data, u32 adr);
u32 gdb_prefetch32(void *data, u32 adr);
u8 gdb_read8(void *data, u32 adr);
u16 gdb_read16(void *data, u32 adr);
u32 gdb_read32(void *data, u32 adr);
void gdb_write8(void *data, u32 adr, u8 val);
void gdb_write16(void *data, u32 adr, u16 val);
void gdb_write32(void *data, u32 adr, u32 val);
