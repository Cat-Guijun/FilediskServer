#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>
#include <wfrest/HttpServer.h>
#include <workflow/MySQLResult.h>
#include <wfrest/Json.h>
#include <iostream>
using namespace std;

using namespace wfrest;
static WFFacilities::WaitGroup wait_group(1);


int main(){
    // auto task= WFTaskFactory::create_mysql_task("mysql://root:131402@localhost:3306",10,
    // [](WFMySQLTask * mytask){
    //     std::cout<<"xxxx"<<std::endl;
    //     protocol::MySQLResponse * resp=mytask->get_resp();
    //     protocol::MySQLResultCursor cursor(resp);
    //     // cursor.get_cursor_status() == MYSQL_STATUS_OK
    //     std::cout << cursor.get_cursor_status()<<endl; // MYSQL_STATUS_NOT_INIT	=	0,
    //     std::vector<protocol::MySQLCell> row; 
    //     cursor.fetch_row(row);
    //     for(auto &cell:row){
    //         std::cout<<cell.as_string()<<std::endl;
    //     }
    // });
    // string sql= "insert into pracdemo.class values(1,'1111111', 100);";
    // task->get_req()->set_query(sql);
    // task->start();
    wfrest::Json json;
    json["id"]=1;
    json["user"]="kkk";
    cout<< json["user"] << endl;
    wait_group.wait();
}