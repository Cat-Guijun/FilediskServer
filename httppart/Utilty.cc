#include "workflow/WFFacilities.h"
#include <workflow/WFTaskFactory.h>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
using namespace std;

class Utility{
public:
	inline static void redis_session_task(string uuid,unsigned long long userid,string username){
		WFRedisTask* redtask=WFTaskFactory::create_redis_task("redis://127.0.0.1:6379",10,
			[](WFRedisTask* red){});
		auto here=redtask->get_req();
		here->set_request("hmset",{uuid,"username",username,"userid",to_string(userid)});
		redtask->start();
	}
    static string gen_uuid(){
		// gen uuid
		boost::uuids::random_generator gen;
    	boost::uuids::uuid uuid = gen();
    	return boost::uuids::to_string(uuid);
    }
	static string gen_salt(){
		unsigned char charset[]="ashdfdfisdlfdmfdk112fvn2vjdsdsjdjvjdaodsaowofdfdspq1232";
		srand(time(nullptr));
		char salt[16];
		int len= sizeof(charset);
		for(int i=0;i<16;i++){
			salt[i]=charset[rand()%len];
		}
		return string(salt,16);
	}

};