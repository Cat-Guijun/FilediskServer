#include "UserSign.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

static void signup_done(UserRet *response, srpc::RPCContext *context)
{
}

static void signin_done(UserRet *response, srpc::RPCContext *context)
{
}

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	const char *ip = "127.0.0.1";
	unsigned short port = 1412;

	UserSignUpService::SRPCClient client(ip, port);
	// UserSignInService::SRPCClient client1(ip, port)

	// example for RPC method call
	UserSign signup_req;
	//signup_req.set_message("Hello, srpc!");
	client.SignUp(&signup_req, signup_done);

	UserSignInService::SRPCClient client1(ip, port);

	// example for RPC method call
	UserSign signin_req;
	//signin_req.set_message("Hello, srpc!");
	client1.SignIn(&signin_req, signin_done);

	wait_group.wait();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
