Getting Started with ware_module
=======

| 版本号      |  时间    | 作者   | 更新内容 |
| -------- | ------ | -------- |  :------ |
2.0.22|2019.8.6|zhijun.yao|基于X1的IPC3.2识别模型及X2的V1.2模型提供底库管理和检索的能力
***

# 功能设计

ware_module SDK提供底库管理和底库检索的功能,在xwarehouse的基础上增加库容限制的功能。
***

# 接口设计

对外提供C接口

## 数据结构

接口使用的数据接口定义在ware_module.h头文件中，主要数据接口如下：

一些常量定义：

```ruby
//id字符串长度限制
#define ID_MAX_LENGTH (48)
//uri字符串长度限制
#define URI_MAX_LENGTH (256)
//文件目录字符串长度限制
#define PATH_MAX_LENGTH (128)
// 版本信息长度限制
#define VERSION_MAX_LENGTH (256)
```
错误码：HobotXWHErrCode
```ruby
WARE_OK = 0,  // ok
WARE_PARAM_ERR = 1000,              // 参数错误,字段为空，指针为空等
WARE_INIT_FAIL = 1001,              // 初始化失败
WARE_CREATE_TABLE_FAIL = 1002,      // 插入set信息失败
WARE_DELETE_TABLE_FAIL = 1003,      // 删除set信息失败
WARE_UPDATE_THRESHOLD_FAIL = 1004,  // 更新阈值失败
WARE_LIST_TABLE_FAIL = 1005,        // list set失败
WARE_LOAD_TABLE_FAIL = 1006,        // 加载数据库中的set信息失败
WARE_SOURCETYPE_NOT_MATCH = 1007,   // 类型不匹配
WARE_TABLE_INIT_FAIL = 1010,        // 分库初始化失败
WARE_TABLE_EXIST = 1011,            // 分库已经存在
WARE_TABLE_NOT_EXIST = 1012,        // 分库不存在
WARE_FEATURE_SIZE_NOT_MATCH = 1013, // 特征向量大小不匹配
WARE_ADD_RECORD_FAIL = 1014,        // 新增记录失败
WARE_DELETE_RECORD_FAIL = 1015,     // 删除记录失败
WARE_UPDATE_RECORD_FAIL = 1016,     // 更新记录失败
WARE_ADD_FEATURE_FAIL = 1017,       // 新增特征失败
WARE_DELETE_FEATURE_FAIL = 1018,    // 删除特征失败
WARE_UPDATE_FEATURE_FAIL = 1019,    // 更新特征失败
WARE_ID_EXIST = 1020,               // id已经存在
WARE_ID_NOT_EXIST = 1021,           // id不存在
WARE_DECRYPT_FAIL = 1022,           // 特征值解密失败
WARE_OVER_LIMIT = 1023,             // 超过库容
WARE_GET_LIMIT_FAIL = 1024,         // 获取库容大小失败
WARE_GET_RECORD_FAIL = 1025,        // 获取记录失败
WARE_NOT_INIT = 1026,               // ware_module未初始化
```

**1、ware_store_t：数据源类型**
```ruby
/**
 * @brief 底库加载的数据源类型
 */
typedef enum {
    /* embedded database, load into memory and database */
    WARE_SQLITE = 1,
    /* only load into memory */
    WARE_MEMORY = 3,
} ware_store_t;
```
**2、ware_table_t:数据源信息**

|  字段名称   | 类型  |  说明  |
| --------- | ----- | :----- |
table_name|char[]|分库名
model_version|char[]|模型名

```ruby
typedef struct {
    /* table name, ID */
    char    table_name[ID_MAX_LENGTH];
    /* model version, accord to actual */
    char    model_version[VERSION_MAX_LENGTH];
} ware_table_t;
```

**3、ware_feature_t:单个特征**

单个特征保存的结构体

|  字段名称   | 类型  |  说明  |
| --------- | ----- | :----- |
img_uri|char[]|特征对应的uri,除了特征检索的时候可以为空，其他时候必须设置
attr|int|特征属性，bit0等于0为定点，等于1为浮点，bit1~bit5表示shift值, bit6等于0表示加密，等于1表示非加密，bit7~bit31 reserved
size|int|特征向量的大小，128
feature|void *|特征向量数组指针

```ruby
/**
 * @brief 特征
 */
typedef struct {
    /* key for feature, can empty */
    char    img_uri[URI_MAX_LENGTH];
    /* feature attribute.
     * bit0: feature type, 0-int, 1-float;
     * bit1~5: shift value for int type
     * bit6: 0-feature is encrypted; 1-not encrypted
     * bit7~bit31 is reserved now
     */
    /*
     * Fixed point encryption:   attr = (shift << 1)
     * Float point encryption:   attr = 1
     * Fixed point unencryption: attr = (1 << 6) | (shift << 1)
     * Float point unencryption: attr = (1 << 6) | 1
     */
    int     attr;
    /* feature vector size */
    int     size;
    /* feature vector value */
    void   *feature;
} ware_feature_t;
```

**4、ware_record_t:记录(包含多个特征)**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
record_id|char[]|记录id
num|int|特征个数
features|ware_feature_t *|特征数组指针

```ruby
typedef struct {
    /* record id */
    char    record_id[ID_MAX_LENGTH];
    /* feature number */
    int     num;
    /* feature value */
    ware_feature_t *features;
} ware_record_t;
```

**5、ware_ftr_list_t:获取记录下特征返回的结果**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
ware_record|ware_record_t|记录

```ruby
typedef struct {
    /* feature list of single record */
    ware_record_t ware_record;
} ware_ftr_list_t;
```

**6、ware_record_list_t:获取分库下记录返回的结果**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
num|int|记录个数
record|ware_record_t *|记录

```ruby
typedef struct {
    /* record number */
    int num;
    /* record data */
    ware_record_t *record;
} ware_record_list_t;
```

**7、ware_score_t ：检索出来的单个结果信息**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
id|char[]| 匹配到的id
distance|float| 距离
similar|float| 相似度

```ruby
/**
 * @brief 检索的结果信息
 */
typedef struct {
    /* match ID */
    char    id[ID_MAX_LENGTH];
    /* result distance */
    float   distance;
    /* result similar */
    float   similar;
} ware_score_t;
```
**8、ware_search_t ：检索结果**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
match|bool| 是否匹配到
num|int| 返回的结果个数
id_score|ware_score_t*|ware_score_t数组指针

```ruby
/**
 * @brief 检索的结果
 */
typedef struct {
    /* search result */
    bool          match;
    /* result number */
    int           num;
    /* result detail */
    ware_score_t  *id_score;
} ware_search_t;
```

**9、ware_compare_t ：检索结果**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
match|bool| 是否匹配到
distance|float| 距离
similar|float| 相似度

```ruby
/**
 * @brief 检索的结果
 */
typedef struct {
    /* compare result */
    bool    match;
    /* result distance */
    float   distance;
    /* result similar */
    float   similar;
} ware_compare_t;
```

**10、ware_param_t ：检索参数**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
display_top_num|int|需要返回topN的检索结果
distance_threshold|float|距离阈值
similar_threshold|float|相似度计算参数
num|int|待检索的特征数组大小
features|ware_feature_t *|待检索的特征数组指针

```ruby
typedef struct {
    /* search match number */
    int             display_top_num;
    /* distance threshold, use default set -1 */
    float           distance_threshold;
    /* similar threshold, use default can set -1 */
    float           similar_threshold;
    /* feature number */
    int             num;
    /* feature value */
    ware_feature_t  *features;
} ware_param_t;
```

**11、ware_table_info_t : 分库信息**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
table_name|char[]| 分库名称
model_version|char[]|模型版本
feature_size|int| 分库中保存的特征向量大小
distance_threshold|float|分库的距离阈值
similar_threshold|float|分库的相似度计算参数
database_path|char[]| 该分库对应的db文件
check_flag|int|是否校验属性

```ruby
typedef struct {
    /* table name */
    char    table_name[ID_MAX_LENGTH];
    /* model version */
    char    model_version[VERSION_MAX_LENGTH];
    /* feature size,128 or 256 */
    int     feature_size;
    /* distance threshold */
    float   distance_threshold;
    /* similar threshold */
    float   similar_threshold;
    /* directory path of table */
    char    database_path[PATH_MAX_LENGTH];
    /* Whether or not to check attributes (currently only check race),0: no check,1: check race attributes */
    int     check_flag;
} ware_table_info_t;
```
**12、ware_table_list_t ：获取分库信息返回的结果**

| 字段名称 | 类型  | 说明  |
| ----- | ---- | :--- |
num|int| 分库个数
table_info|ware_table_info_t *|分库数组

```ruby
/**
 * @brief set信息
 */
typedef struct {
  /* table number */
  int num;
  /* table detail */
  ware_table_info_t *table_info;
} ware_table_list_t;
```

## 对外接口

对外接口的定义在ware_module.h头文件中

**1、ware_module_get_version**

获取SDK版本信息

```ruby
/**
 * @brief ware_module_get_version : get ware module version
 *
 * @return char * : version string buffer
 */
char *ware_module_get_version(void);
```

**2、ware_module_init**

初始化接口，指定数据源信息，
当底库信息和记录保存在sqlite中时，如果目录下没有db文件，该接口会创建一个保存分库信息的db文件(warehouse.db)，如果目录下存在该db文件，则会加载该db文件并加载每个分库的记录

```ruby
/**
 * @brief ware_module_init : init warehouse module,create primary database
 *
 * @param [in] db_dir : database and ware_param.bin config file directory
 * @param [in] store_type : database store type
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_init(char *db_dir, ware_store_t store_type);
```

**3、ware_module_deinit**

关闭SDK，释放资源

```ruby
/**
 * @brief ware_module_deinit : deinit warehouse module, release all resource
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_deinit(void);
```

**4、ware_module_create_table**

新增一个分库,当数据源是sqlite时，该接口会在传入的目录下生成一个db文件，该文件会保存所有的记录

```ruby

/**
 * @brief ware_module_create_table : create a table(secondary database)
 *
 * @param [in] table_path : table directory
 * @param [in] store_type : database store type
 * @param [in] table : table information
 * @param [in] feature_size : origin feature size:128/256
 * @param [in] check_flag : WARE_CHECK_NULL, WARE_CHECK_RACE

 * @return int : status code of ware_error_t
 */
int32_t ware_module_create_table(char *table_path,
                                 ware_store_t store_type,
                                 ware_table_t *table,
                                 int32_t feature_size,
                                 ware_check_flag_t check_flag);

```


**5、ware_module_destroy_table**

删除分库，会删除对应的db文件

```ruby
/**
 * @brief ware_module_destroy_table : delete a table(secondary database)
 *
 * @param [in] table : table information
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_destroy_table(ware_table_t *table);
```

**6、ware_module_list_table**

列出所加载的分库，调用完要释放*table_list

```ruby
/**
 * @brief ware_module_list_table : list all table(secondary database)
 *
 * @param [in] table_list : table list
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_list_table(ware_table_list_t **table_list);
```

**7、ware_module_release_table_list**

释放ware_table_list_t指针，调用完ware_module_list_table接口后必须调用

```ruby
/**
 * @brief ware_module_release_table_list : release list pointer
 *
 * @param [in] table_list : table list
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_release_table_list(ware_table_list_t **table_list);
```

**8、ware_module_set_threshold**

设置分库阈值, 兼容旧接口，但不能调参

```ruby

/**
 * @brief ware_module_set_threshold : set threshold of table
 *
 * @param [in] ware_table : table information
 * @param [in] distance_threshold : distance threshold
 * @param [in] similar_threshold : calculate parameter for similar
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_set_threshold(ware_table_t *ware_table,
                                  float distance_threshold,
                                  float similar_threshold);
```

**9、ware_module_add_feature**

指定分库指定记录新增一个特征

```ruby
/**
 * @brief ware_module_add_feature : add feature to a specific record
 *
 * @param [in] ware_table : table information
 * @param [in] record_id : record id
 * @param [in] ware_feature : feature data
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_add_feature(ware_table_t *ware_table,
                                char *record_id,
                                ware_feature_t *ware_feature);
```

**10、ware_module_delete_feature**

删除指定记录指定的特征

```ruby
/**
 * @brief ware_module_delete_feature : delete feature from a specific record
 *
 * @param [in] ware_table : table information
 * @param [in] record_id : record id
 * @param [in] feature_uri : specified feature url
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_delete_feature(ware_table_t *ware_table,
                                   char *record_id,
                                   char *feature_uri);

```

**11、ware_module_update_feature**

更新指定记录的指定的特征

```ruby
/**
 * @brief ware_module_update_feature : delete feature from a specific record
 *
 * @param [in] ware_table : table information
 * @param [in] record_id : record id
 * @param [in] ware_feature : feature data
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_update_feature(ware_table_t *ware_table,
                                   char *record_id,
                                   ware_feature_t *ware_feature);
```

**12、ware_module_list_feature**

列出记录下特征

```ruby
/**
 * @brief ware_module_list_feature : list all feature from a specific record
 *
 * @param [in] ware_table : table information
 * @param [in] record_id : record id
 * @param [in] list : feature list
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_list_feature(ware_table_t *ware_table,
                                 char *record_id,
                                 ware_ftr_list_t **list);
```

**13、ware_module_release_feature_list**

释放内存，调用完ware_module_list_feature调用

```ruby
/**
 * @brief ware_module_list_feature : list all feature from a specific record
 *
 * @param [in] ware_table : table information
 * @param [in] record_id : record id
 * @param [in] list : feature list
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_release_feature_list(ware_ftr_list_t **list);
```

**14、ware_module_add_record**

指定分库中新增一条记录

```ruby
/**
 * @brief ware_module_add_record : add record data to a specific table
 *
 * @param [in] ware_table : table information
 * @param [in] ware_record : record data
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_add_record(ware_table_t *ware_table, ware_record_t *ware_record);
```

**15、ware_module_delete_record**

删除记录

```ruby
/**
 * @brief ware_module_delete_record : delete a record from a specific table
 *
 * @param [in] ware_table : table information
 * @param [in] record_id : record id
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_delete_record(ware_table_t *ware_table, char *record_id);
```

**16、ware_module_list_record**

列出指定分库下所有的记录

```ruby
/**
 * @brief ware_module_list_record : list all record of a specific table
 *
 * @param [in] ware_table : table information
 * @param [in] list : record list
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_list_record(ware_table_t *ware_table, ware_record_list_t **list);
```

**17、ware_module_release_record_list**

释放内存，调用完ware_module_list_record后调用

```ruby
/**
 * @brief ware_module_release_record_list : release list pointer
 *
 * @param [in] list : record list
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_release_record_list(ware_record_list_t **list);

```

**18、ware_module_update_record**

更新记录

```ruby
/**
 * @brief ware_module_update_record : update a specified record data
 *
 * @param [in] ware_table : table information
 * @param [in] ware_record : record data
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_update_record(ware_table_t *ware_table, ware_record_t *ware_record);
```


**19、ware_module_alloc_search**

申请检索结果内存，多次检索请求调用一次就够了，检索前调用

```ruby
/**
 * @brief ware_module_alloc_search : alloc search result memory
 *
 * @return ware_search_t * : a pointer to ware_search_t
 */
ware_search_t *ware_module_alloc_search(void);
```

**20、ware_module_search_record**

检索

```ruby

/**
 * @brief ware_module_search_record : search a record from a specified table
 *
 * @param [in] ware_table : table information
 * @param [in] ware_param : search parameter
 * @param [in] ware_search : search result
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_search_record(ware_table_t *ware_table,
                                  ware_param_t *ware_param,
                                  ware_search_t *ware_search);
```

**21、ware_module_release_record_search**

释放内存，不需要每次调用完检索后调用

```ruby

/**
 * @brief ware_module_release_record_search : release search result pointer
 *
 * @param [in] ware_table : table information
 * @param [in] ware_param : search parameter
 * @param [in] ware_search : search result
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_release_record_search(ware_search_t **ware_search);
```

**22、ware_module_compare_record**

1比1比对接口

```ruby

//**
  * @brief ware_module_compare_record : compare record
  *
  * @param [in] ware_record1 : record data
  * @param [in] ware_record2 : record data
  * @param [in] distance_threshold : distance threshold, as a calculate factors
  * @param [in] similar_threshold : similar threshold, as a calculate factors
  * @param [in] compare : compare result
  *
  * @return int : status code of ware_error_t
  */
 int32_t ware_module_compare_record(ware_record_t *ware_record1,
                                    ware_record_t *ware_record2,
                                    float distance_threshold,
                                    float similar_threshold,
                                    ware_compare_t *compare);
```

**23、ware_module_check_model_version**

检测识别模型接口

```ruby

/**
 * @brief ware_module_check_model_version : features whether re-register
 *
 * @param [in] current_model_version : current model version
 * @param [in] last_model_version : last model version
 * @param [out] is_update_db : true or false
 *
 * @return int : status code of ware_error_t
 */
int32_t ware_module_check_model_version(const char *current_model_version,
                                        const char *last_model_version,
                                        bool *is_update_db);
```

# 编译

```ruby
X1600平台   ./cicd/scipts/build_release.sh x1600
Hisi平台    ./cicd/scipts/build_release.sh hisi
hisiv500   ./cicd/scipts/build_release.sh hisiv500
```

# Usage

详细的可以参考ware_sample.c

# 技术支持

请联系zhijun.yao@horizon.ai, yongchao.peng@horizon.ai