#include "UserSign.srpc.h"
#include "Utilty.cc"
#include <openssl/md5.h>
#include "workflow/WFFacilities.h"
#include <workflow/WFTaskFactory.h>
#include <workflow/MySQLResult.h>
#include <sstream>

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class UserSignUpServiceServiceImpl : public UserSignUpService::Service
{
public:

	void SignUp(UserSign *request, UserRet *response, srpc::RPCContext *ctx) override
	{
		string username=request->username();
		string password= request->password();
		string salt= Utility::gen_salt();
		unsigned char md[16];
		string p_s=password+salt;
		MD5((const unsigned char*)p_s.c_str(),p_s.size(),md);
		string encpwd((char*)md,16);
		std::ostringstream oss;
		oss<< 
		"insert into user(username,salt,encpwd) values ('" << username << "','" << salt << "','"
		<<  encpwd << "');"; 
		string sql= oss.str();
		auto mysqltask=WFTaskFactory::create_mysql_task("mysql://root:131402@localhost:6379",
			10,
			[response,username](WFMySQLTask* sqltask){
				protocol::MySQLResultCursor cursor(sqltask->get_resp());
				if(cursor.get_cursor_status()==MYSQL_STATUS_OK){
					unsigned long long userid= cursor.get_insert_id();
					string username=username;
					string uuid=Utility::gen_uuid();
					Utility::redis_session_task(uuid,userid,username);
					response->set_code(0);
					response->set_msg("ok!");
					response->set_uuid(uuid);
				}else{
					response->set_code(-1);
					response->set_msg("duplicated username");
				}
			});
		mysqltask->get_req()->set_query(sql);
		ctx->get_series()->push_back(mysqltask);
	}
};
class UserSignInServiceServiceImpl : public UserSignInService::Service
{
public:

	void SignIn(UserSign *request, UserRet *response, srpc::RPCContext *ctx) override
	{
		string username=request->username();
		string password= request->password();
		// unsigned char md[16];
		// string p_s=password+salt;
		// MD5((const unsigned char*)p_s.c_str(),p_s.size(),md);
		// string encpwd((char*)md,16);
		std::ostringstream oss;
		oss<< "select * from user where username='"+ username + "';";
		string sql= oss.str();
		auto mysqltask=WFTaskFactory::create_mysql_task("mysql://root:131402@localhost:6379",
			10,
			[response,username,password](WFMySQLTask* sqltask){
				protocol::MySQLResultCursor cursor(sqltask->get_resp());
				if(cursor.get_cursor_status()==MYSQL_STATUS_OK && cursor.get_affected_rows()>0){
					std::vector<protocol::MySQLCell> row_arr;
					cursor.fetch_row(row_arr);
					unsigned long long userid= row_arr[0].as_ulonglong();
					string username=row_arr[1].as_string();
					string salt=row_arr[2].as_string();
					string encpwd=row_arr[3].as_string();
					unsigned char md[16];
					string p_s=password+salt;
					MD5((const unsigned char*)p_s.c_str(),p_s.size(),md);
					string encpwd2((char*)md,16);
					bool flag=true;
					for(int i=0;i<16;i++){
						if(encpwd2[i]!=encpwd[i]) flag=false;
					}
					if(flag){
						string uuid=Utility::gen_uuid();
						Utility::redis_session_task(uuid,userid,username);
						response->set_code(0);
						response->set_msg("ok!");
						response->set_uuid(uuid);
					}else{
						response->set_code(-1);
						response->set_msg("login error username");
					}

				}else{
					response->set_code(-1);
					response->set_msg("login error username");
				}
			});
		mysqltask->get_req()->set_query(sql);
		ctx->get_series()->push_back(mysqltask);
	}
};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	unsigned short port = 7777;
	SRPCServer server;

	UserSignUpServiceServiceImpl usersignupservice_impl;
	server.add_service(&usersignupservice_impl);

	UserSignInServiceServiceImpl usersigninservice_impl;
	server.add_service(&usersigninservice_impl);

	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
