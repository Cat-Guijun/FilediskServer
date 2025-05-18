#include "FileManager.srpc.h"
#include "workflow/WFFacilities.h"
#include <workflow/WFTaskFactory.h>
#include <workflow/MySQLResult.h>
#include <wfrest/Json.h>
#include <string>
#include <sstream>
using namespace std;

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class DirAddServiceServiceImpl : public DirAddService::Service
{
public:

	void DirAdd(DirInfo *request, DirRet *response, srpc::RPCContext *ctx) override
	{
		unsigned long long dirid=request->dirid();  // parent id
		string dirname= request->name();   // new name
		string uuid= request->uuid();
		using namespace placeholders;
		auto redistask=WFTaskFactory::create_redis_task("redis://127.0.0.1:6379",10,
			std::bind(&redis_callback,response,dirname,dirid,ctx,_1));
		redistask->get_req()->set_request("hgetall",{uuid});
		ctx->get_series()->push_back(redistask);
	}


	static void redis_callback(DirRet *response,string dirname,unsigned long long dirid,srpc::RPCContext *ctx,WFRedisTask *task){
		auto resp=task->get_resp();
		protocol::RedisValue redisvalue;
		resp->get_result(redisvalue);
		if(redisvalue.is_array()){
			string username= redisvalue[0].string_value();
			string userid= redisvalue[1].string_value();
			auto mysqltask= WFTaskFactory::create_mysql_task(
				"mysql://root:131402@127.0.0.1:6379",
				10,
				[response,dirname](WFMySQLTask *task){
					protocol::MySQLResultCursor cursor(task->get_resp());
					if(cursor.get_cursor_status()==MYSQL_STATUS_OK){
						// {"code" : int , "data" : 
						// {"dirid":11,"name":}
						wfrest::Json json;
						json["code"]=0;
						json["dirid"]= cursor.get_insert_id();
						json["name"]=dirname;
						response->set_code(0);
						response->set_jsoninfo(json.dump());
					}else{
						wfrest::Json json;
						json["code"]=-1;
						response->set_code(-1);
						response->set_jsoninfo(json.dump());
					}
				}
			);
			std::ostringstream oss;
			oss<< "insert into filetree(userid,filepath,dirid) values(" +  userid + ","
			+ dirname + std::to_string(dirid) + "," + ");";
			std::string sql=oss.str();
			mysqltask->get_req()->set_query(sql);
			ctx->get_series()->push_back(mysqltask);
		}else{
			std::cout<< "xxxxxxxxx===============xxxxxxxxx redis in queryfile\n";
		}
	}

};
class DirRmServiceServiceImpl : public DirRmService::Service
{
public:

	void DirRm(DirInfo *request, DirRet *response, srpc::RPCContext *ctx) override
	{
		unsigned long long dirid=request->dirid();  // itself id
		string dirname= request->name();
		string uuid= request->uuid();
		using namespace placeholders;
		auto redistask=WFTaskFactory::create_redis_task("redis://127.0.0.1:6379",10,
			std::bind(&redis_callback,response,dirname,dirid,ctx,_1));
		redistask->get_req()->set_request("hgetall",{uuid});
		ctx->get_series()->push_back(redistask);
	}

	static void redis_callback(DirRet *response,string dirname,unsigned long long dirid,srpc::RPCContext *ctx,WFRedisTask *task){
		auto resp=task->get_resp();
		protocol::RedisValue redisvalue;
		resp->get_result(redisvalue);
		if(redisvalue.is_array()){
			string username= redisvalue[0].string_value();
			string userid= redisvalue[1].string_value();
			auto mysqltask= WFTaskFactory::create_mysql_task(
				"mysql://root:131402@127.0.0.1:6379",
				10,
				[response](WFMySQLTask *task){
					protocol::MySQLResultCursor cursor(task->get_resp());
					if(cursor.get_cursor_status()==MYSQL_STATUS_OK && cursor.get_affected_rows()==1){
						// {"code" : int , "data" :
						wfrest::Json json;
						json["code"]=0;
						json["msg"]=string("del ok!");
						response->set_code(0);
						response->set_jsoninfo(json.dump());
					}else{
						wfrest::Json json;
						json["code"]=-1;
						json["msg"]=string("del error");
						response->set_code(-1);
						response->set_jsoninfo(json.dump());
					}
				}
			);
			std::ostringstream oss;
			oss<< "update filetree set dirid=-2 where userid= " + userid + 
			"and dirid=" + std::to_string(dirid) + ";";
			std::string sql=oss.str();
			mysqltask->get_req()->set_query(sql);
			ctx->get_series()->push_back(mysqltask);
		}else{
			std::cout<< "xxxxxxxxx===============xxxxxxxxx redis in queryfile\n";
		}
	}


};
class DirQueryServiceServiceImpl : public DirQueryService::Service
{
public:

	void DirQuery(DirInfo *request, DirRet *response, srpc::RPCContext *ctx) override
	{
		unsigned long long dirid=request->dirid();  // itself id
		string dirname= request->name();
		string uuid= request->uuid();
		using namespace placeholders;
		auto redistask=WFTaskFactory::create_redis_task("redis://127.0.0.1:6379",10,
			std::bind(&redis_callback,response,dirname,dirid,ctx,_1));
		redistask->get_req()->set_request("hgetall",{uuid});
		ctx->get_series()->push_back(redistask);
	}

	static void redis_callback(DirRet *response,string dirname,unsigned long long dirid,srpc::RPCContext *ctx,WFRedisTask *task){
		auto resp=task->get_resp();
		protocol::RedisValue redisvalue;
		resp->get_result(redisvalue);
		if(redisvalue.is_array()){
			string username= redisvalue[0].string_value();
			string userid= redisvalue[1].string_value();
			auto mysqltask= WFTaskFactory::create_mysql_task(
				"mysql://root:131402@127.0.0.1:6379",
				10,
				[response](WFMySQLTask *task){
					protocol::MySQLResultCursor cursor(task->get_resp());
					if(cursor.get_cursor_status()==MYSQL_STATUS_OK){
						std::vector<std::vector<protocol::MySQLCell>> rows;
						cursor.fetch_all(rows);
						// {"code" : int , "data" : 
						// [{"id":11,"filesize":0,"filename":xx, "index":0 },{}]}
						wfrest::Json json;
						json["code"]=0;
						wfrest::Json::Array data;
					//	vector<wfrest::Json> data;
						for(auto &row :rows) {
							wfrest::Json::Object rowjson;
							// for(auto &cell:row){
							rowjson["id"] = row[0].as_ulonglong();
							rowjson["name"] = row[1].as_string();
							rowjson["index"] = row[2].as_int();
							rowjson["filesize"] = row[3].as_ulonglong();
							data.push_back(rowjson);
						}
						json["data"]=data;
						response->set_code(0);
						response->set_jsoninfo(json.dump());
					}else{
						wfrest::Json json;
						json["code"]=-1;
						response->set_code(-1);
						response->set_jsoninfo(json.dump());
					}
				}
			);
			std::ostringstream oss;
			oss<< "select (id,filepath as name,index,filesize) from filetree where userid= " + userid + 
			"and dirid=" + std::to_string(dirid) + ";";
			std::string sql=oss.str();
			mysqltask->get_req()->set_query(sql);
			ctx->get_series()->push_back(mysqltask);
		}else{
			std::cout<< "xxxxxxxxx===============xxxxxxxxx redis in queryfile\n";
		}
	}

};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	unsigned short port = 1412;
	SRPCServer server;

	DirAddServiceServiceImpl diraddservice_impl;
	server.add_service(&diraddservice_impl);

	DirRmServiceServiceImpl dirrmservice_impl;
	server.add_service(&dirrmservice_impl);

	DirQueryServiceServiceImpl dirqueryservice_impl;
	server.add_service(&dirqueryservice_impl);

	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
