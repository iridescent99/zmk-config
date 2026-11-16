#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "fakes.h"
/* Test the actual startup adapter, including its private callbacks and guard. */
#include "../../boards/shields/sharp_mip/widgets/split_repair.c"

struct record { char path[80]; unsigned char bytes[64]; size_t len; };
static struct record records[40];
static size_t record_count, mutation_count, delete_count;
static char mutations[16][80];
static bool fail_marker;
static int fail_delete;
static const char *fail_read;
static const bt_addr_le_t peer = {.type=1, .a={.val={0x66,0x55,0x44,0x33,0x22,0x11}}};
static const bt_addr_le_t host = {.type=0, .a={.val={0xff,0xee,0xdd,0xcc,0xbb,0xaa}}};

static struct record *lookup(const char *path) {
    for (size_t i=0;i<record_count;i++) { if (!strcmp(path,records[i].path)) return &records[i]; }
    return NULL;
}
static void put(const char *path,const void *value,size_t len) {
    struct record *r=lookup(path);
    if (!r) { assert(record_count<40); r=&records[record_count++]; strcpy(r->path,path); }
    assert(len<=sizeof(r->bytes)); memcpy(r->bytes,value,len); r->len=len;
}
static void trace(const char *path) {
    assert(mutation_count<16); strcpy(mutations[mutation_count++],path);
}
int settings_subsys_init(void) { return 0; }
bool settings_name_steq(const char *key,const char *name,const char **next) {
    size_t n=strlen(name);
    if (strncmp(key,name,n) || (key[n] && key[n]!='/')) return false;
    *next=key[n]=='/'?key+n+1:NULL; return true;
}
static ssize_t read_value(void *ctx,void *dst,size_t n) {
    struct record *r=ctx;
    if (fail_read && !strcmp(fail_read,r->path)) return -EIO;
    if (n>r->len) n=r->len;
    memcpy(dst,r->bytes,n); return (ssize_t)n;
}
int settings_load_subtree_direct(const char *subtree,settings_load_direct_cb cb,void *param) {
    for (size_t i=0;i<record_count;i++) {
        const char *next;
        if (records[i].len && settings_name_steq(records[i].path,subtree,&next)) {
            if (cb(next,records[i].len,read_value,&records[i],param)) break;
        }
    }
    /* Match the real Zephyr 3.5 backend wrapper: callback errors are swallowed. */
    return 0;
}
int settings_save_one(const char *path,const void *value,size_t len) {
    trace(path);
    assert(!strcmp(path,ATTEMPT_KEY));
    if (fail_marker) return -EIO;
    put(path,value,len); return 0;
}
int settings_delete(const char *path) {
    trace(path);
    if ((int)delete_count++==fail_delete) return -EIO;
    struct record *r=lookup(path);
    if (!r) return -ENOENT;
    r->len=0; return 0;
}
static void setup(void) {
    memset(records,0,sizeof(records)); record_count=mutation_count=delete_count=0;
    fail_marker=false; fail_delete=-1; fail_read=NULL; result="FIX ERR";
    const char *paths[]={"bt/id","bt/irk","ble/active_profile","keymap/l/0/0",
        "physical_layouts/selected","rgb/underglow","bt/keys/1122334455661",
        "bt/ccc/1122334455661","bt/cf/1122334455661","bt/sc/1122334455661"};
    for (size_t i=0;i<sizeof(paths)/sizeof(paths[0]);i++) put(paths[i],"x",1);
#if CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    struct zmk_ble_profile p={.peer=host};
    put("ble/profiles/0",&p,sizeof(p));
    put("ble/peripheral_addresses/0",&peer,sizeof(peer));
    put("bt/keys/aabbccddeeff0","host-secret",11);
    put("bt/ccc/aabbccddeeff0","host-ccc",8);
#endif
}
static void preserved(void) {
    const char *paths[]={"bt/id","bt/irk","ble/active_profile","keymap/l/0/0",
                        "physical_layouts/selected","rgb/underglow"};
    for (size_t i=0;i<sizeof(paths)/sizeof(paths[0]);i++) {
        struct record *r=lookup(paths[i]); assert(r && r->len==1 && r->bytes[0]=='x');
    }
#if CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    assert(lookup("bt/keys/aabbccddeeff0")->len==11);
    assert(lookup("bt/ccc/aabbccddeeff0")->len==8);
    assert(lookup("ble/profiles/0")->len==sizeof(struct zmk_ble_profile));
    assert(!memcmp(lookup("ble/peripheral_addresses/0")->bytes,&peer,sizeof(peer)));
#endif
}
int main(void) {
    setup(); assert(!repair_init()); assert(!strcmp(result,"FIX DONE"));
    assert(mutation_count==5 && !strcmp(mutations[0],ATTEMPT_KEY));
    assert(delete_count==4); preserved();
    assert(!repair_init() && !strcmp(result,"FIX USED") && mutation_count==5);

    setup(); fail_marker=true; repair_init();
    assert(!strcmp(result,"FIX ERR") && delete_count==0 && !lookup(ATTEMPT_KEY)); preserved();
    setup(); fail_delete=1; repair_init();
    assert(!strcmp(result,"FIX ERR") && delete_count==2); preserved();
    repair_init(); assert(!strcmp(result,"FIX USED") && delete_count==2);

    setup(); put("bt/keys/1122334455661/0","x",1); repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0); preserved();

#if CONFIG_ZMK_SPLIT_ROLE_CENTRAL
    setup(); put("ble/peripheral_addresses/0",&host,sizeof(host)); repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0);
    setup(); put("ble/profiles/0","bad",3); repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0);
    setup(); fail_read="ble/profiles/0"; repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0); preserved();
    setup(); bt_addr_le_t bad=peer; bad.type=10;
    put("ble/peripheral_addresses/0",&bad,sizeof(bad)); repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0);
#else
    setup(); put("bt/keys/aabbccddeeff0","x",1); repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0); preserved();
    setup(); struct zmk_ble_profile p={.peer=peer};
    put("ble/profiles/0",&p,sizeof(p)); repair_init();
    assert(!strcmp(result,"FIX STOP") && mutation_count==0); preserved();
    setup(); lookup("bt/keys/1122334455661")->len=0; repair_init();
    assert(!strcmp(result,"FIX NONE") && delete_count==0); preserved();
    (void)host;
#endif
    puts("PASS startup adapter: host/identity/Studio preservation, one-shot guard, failures and partial reads");
}
