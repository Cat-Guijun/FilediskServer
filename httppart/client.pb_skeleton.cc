#include "FileManager.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

static void diradd_done(DirRet *response, srpc::RPCContext *context)
{
}

static void dirrm_done(DirRet *response, srpc::RPCContext *context)
{
}

static void dirquery_done(DirRet *response, srpc::RPCContext *context)
{
}

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	const char *ip = "127.0.0.1";
	unsigned short port = 1412;

	DirAddService::SRPCClient client(ip, port);

	// example for RPC method call
	DirInfo diradd_req;
	//diradd_req.set_message("Hello, srpc!");
	client.DirAdd(&diradd_req, diradd_done);

	DirRmService::SRPCClient client1(ip, port);

	// example for RPC method call
	DirInfo dirrm_req;
	//dirrm_req.set_message("Hello, srpc!");
	client1.DirRm(&dirrm_req, dirrm_done);

	DirQueryService::SRPCClient client2(ip, port);

	// example for RPC method call
	DirInfo dirquery_req;
	//dirquery_req.set_message("Hello, srpc!");
	client2.DirQuery(&dirquery_req, dirquery_done);

	wait_group.wait();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
