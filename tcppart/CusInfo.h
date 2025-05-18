
// communication data structure


struct datauuid
{
    char uuid[33]={0}; // 保持session
    int namelen=0;
};

struct fileheader{
    long long dirid;
    unsigned char md5[16];
    int index;  // 分片次序
    bool upload;
    long long filelen;
};

struct UserValidation
{
    bool valid;
};

struct fileexists{
    bool exists;
};

struct fileerror{
    bool error;
};