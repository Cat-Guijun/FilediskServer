# FilediskServer
Based on TCP and HTTP protocols.
epoll is needed!

## cpp library
see it in CMakeLists.txt; 
github mirror: git clone https://kkgithub.com/wfrest/wfrest.git.
git clone https://kkgithub.com/sogou/workflow.git
## database table design
1. table 1

filesize=0 denotes it is a dir.

```
filetree (id,userid,filepath,index,hashfilepath,dirid,filesize)   
create index fileindex(userid,dirid);
default value : hashfilepath "" filesize 0
```

2. table 2
```
create table user(id int primary key auto_increment,
  username varchar(50) not null unique_key,
  salt varchar(16), encpwd varchar(64));

```

## docker download

rabbitmq:
docker run -d \
  --name rabbitmq \
  -p 5672:5672 \
  -p 15672:15672 \
  -v $PWD/rabbitmq_data:/var/lib/rabbitmq \
  rabbitmq:management

consul:



**Upcoming in Consul 1.16, we will stop publishing official Dockerhub images and publish only our Verified Publisher images. Users of Docker images should pull from hashicorp/consul instead of consul.**

docker run -d --name=consul -p 8500:8500 -p 8600:8600/udp consul agent -dev -client=0.0.0.0  



## TCP part function
1. upload:
checkuservalidation
checkfileexits
checkfileok

2. download: sequential download;




## Http part
restful api:
1. /register
note that in workflow/mysql:
std::cout << cursor.get_cursor_status()<<endl; // MYSQL_STATUS_NOT_INIT	=	0,
2. /signin


```
dirid;  parent's
string uuid;
string name;


int code;
string jsoninfo;
```
3. /querydir

4. /removedir

5. /adddir





## srpc outline


 srpc_generator protobuf FileManager.proto .




based on workflow:
1. install : wfrest.
```
git clone https://github.com/wfrest/wfrest.git
cd wfrest
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

2. 
```
install : protobuf
install : 
```

3. brpc
```
git clone https://github.com/sogou/srpc.git
cd srpc
mkdir build && cd build
cmake ..
make -j
sudo make install
```
ps: rpc.proto format:

}
```



```


4. ppconsul
