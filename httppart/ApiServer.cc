#include <wfrest/HttpServer.h>
#include <wfrest/HttpCookie.h>
#include <workflow/WFFacilities.h>
#include <workflow/Workflow.h>
#include <workflow/WFTaskFactory.h>
#include <signal.h>
#include <wfrest/Json.h>
#include "UserSign.srpc.h"
#include "FileManager.srpc.h"
#include "FileManager.pb.h"
#include <string>
using namespace std;
static WFFacilities::WaitGroup wait_group(1);


// struct user_info{
//     std::string username;
//     std::string password;
//     std::string salt;
// };

// struct cus_context{
//     user_info user;
//     wfrest::HttpReq *req;
// };

// rpc 


void query_dir_http(const wfrest::HttpReq *req, wfrest::HttpResp *resp,SeriesWork *series){
    const char *ip = "127.0.0.1";
	unsigned short port = 9999;   // consul search
    DirQueryService::SRPCClient client(ip,port);
    auto task=client.create_DirQuery_task([resp](DirRet *ret,srpc::RPCContext *ctx)
    {
        string jsoninfo=ret->jsoninfo();
        resp->String(jsoninfo);
    });
    DirInfo *info= new DirInfo();
    wfrest::Json json=req->json();
    info->set_dirid(json["dirid"]);
    string name=json["dirName"];
    info->set_name(name);
    string uuid=json["uuid"];
    info->set_uuid(uuid);
    task->serialize_input(info);
    series->push_back(task);
    series->set_callback([info](const SeriesWork * series){
            delete info;
    });
}


void add_dir_http(const wfrest::HttpReq *req, wfrest::HttpResp *resp,SeriesWork *series){
    const char *ip = "127.0.0.1";
	unsigned short port = 9999;   // consul search
    DirAddService::SRPCClient client(ip,port);
    auto task=client.create_DirAdd_task([resp](DirRet *ret,srpc::RPCContext *ctx)
    {
        string jsoninfo=ret->jsoninfo();
        resp->String(jsoninfo);
    });
    DirInfo *info= new DirInfo();
    wfrest::Json json=req->json();
    info->set_dirid(json["dirid"]);
    string name=json["dirName"];
    info->set_name(name);
    string uuid=json["uuid"];
    info->set_uuid(uuid);
    task->serialize_input(info);
    series->push_back(task);
    series->set_callback([info](const SeriesWork * series){
            delete info;
    });
}


void rm_dir_http(const wfrest::HttpReq *req, wfrest::HttpResp *resp,SeriesWork *series){
    const char *ip = "127.0.0.1";
	unsigned short port = 9999;   // consul search
    DirRmService::SRPCClient client(ip,port);
    auto task=client.create_DirRm_task([resp](DirRet *ret,srpc::RPCContext *ctx)
    {
        string jsoninfo=ret->jsoninfo();
        resp->String(jsoninfo);
    });
    DirInfo *info= new DirInfo();
    wfrest::Json json=req->json();
    info->set_dirid(json["dirid"]);
    string name=json["dirName"];
    info->set_name(name);
    string uuid=json["uuid"];
    info->set_uuid(uuid);
    task->serialize_input(info);
    series->push_back(task);
    series->set_callback([info](const SeriesWork * series){
            delete info;
    });
}




int main(void){
    signal(SIGINT, [](int signum) {
        wait_group.done();
    });

    wfrest::HttpServer server;
    server.GET("/index.html",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->add_header("Content-Type", "text/html");
        resp->String("<html><body><h1>Hello, World!</h1></body></html>");
    });

    server.POST("/register",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp,SeriesWork *series){
        std::string username=req->json()["username"];
        std::string password=req->json()["password"];
        GOOGLE_PROTOBUF_VERIFY_VERSION;
	    const char *ip = "127.0.0.1";
	    unsigned short port = 7777;   // consul search

        UserSignUpService::SRPCClient client(ip, port);
        UserSign *signup_req=new UserSign;
        signup_req->set_username(username);
        signup_req->set_password(password);
        auto task=client.create_SignUp_task([resp](UserRet *response, srpc::RPCContext *context){
            resp->headers["Content-Type"] = "application/json";
            wfrest::Json json;
            wfrest::HttpCookie cookie;
            cookie.set_key("uuid").set_value(response->uuid()).set_expires(wfrest::Timestamp::now() + (double)60*24).set_path("/");
            resp->add_cookie(cookie);
            json["status"] = response->code();
            json["msg"] = response->msg();
            resp->String(json.dump());
        });
         task->serialize_input(signup_req);
        series->push_back(task);
        series->set_callback([signup_req](const SeriesWork * series){
            delete signup_req;
        });
    });

    server.POST("/login",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp,SeriesWork *series){
        std::string username=req->json()["username"];
        std::string password=req->json()["password"];
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        const char *ip = "127.0.0.1";
	    unsigned short port = 7777;   // consul search
        UserSignInService::SRPCClient client(ip, port);
        UserSign *signup_req=new UserSign;
        signup_req->set_username(username);
        signup_req->set_password(password);
        auto task=client.create_SignIn_task([resp](UserRet *response, srpc::RPCContext *context){
            resp->headers["Content-Type"] = "application/json";
            wfrest::Json json;
            wfrest::HttpCookie cookie;
            cookie.set_key("uuid").set_value(response->uuid()).set_expires(wfrest::Timestamp::now() + (double)60*24).set_path("/");
            resp->add_cookie(cookie);
            json["status"] = response->code();
            json["msg"] = response->msg();
            resp->String(json.dump());
        });
        task->serialize_input(signup_req);
        series->push_back(task);
        series->set_callback([signup_req](const SeriesWork * series){
            delete signup_req;
        });
    });
    
    server.POST("/querydir",query_dir_http);
    server.POST("/adddir",add_dir_http);
    server.POST("/rmdir",rm_dir_http);


    server.start(1234);
    wait_group.wait();
    server.stop();
}





