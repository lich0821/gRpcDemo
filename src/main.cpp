/*
Application
*/

#include <random>

#include <grpcpp/server_builder.h>

#include "server.hpp"

#ifdef _WIN32
#define SLEEP(x) Sleep((x))
#else
#define SLEEP(x) sleep((x) / 1000)
#endif

mutex gMutex;
queue<WxMsg> gMsgQueue;
condition_variable gCv;

void ProduceMsg(queue<WxMsg> *msg_queue)
{
    random_device rd;  // 随机数生成器
    mt19937 gen(rd()); // Mersenne Twister 算法

    uniform_int_distribution<int> random_i(1000, 5000); // (1000, 5000) 均匀分布

    int i = 0;
    while (true) {
        int tmp = random_i(gen); // 生成 (1000, 5000) 随机数

        // 生成消息
        WxMsg msg;
        msg.set_is_self(false);
        msg.set_is_group(tmp % 2 == 0);
        msg.set_is_self(false);
        msg.set_is_group(tmp % 2 == 0);
        msg.set_type(tmp / 100);
        msg.set_id("Id_" + to_string(i++));
        msg.set_xml("<xml></xml>");
        msg.set_sender("wxid_" + to_string(tmp * tmp));
        msg.set_roomid(to_string(i * tmp) + "@chatroom");
        msg.set_content("Content" + to_string(tmp + tmp));

        // 推送到队列
        unique_lock<std::mutex> locker(gMutex);
        msg_queue->push(msg);
        locker.unlock();

        // 通知各方消息就绪
        gCv.notify_all();

        // 模拟消息到达间隔
        SLEEP(tmp);
    }
}

void ConsumeMsg(queue<WxMsg> *msg_queue)
{
    while (true) {
        unique_lock<std::mutex> lock(gMutex);
        gCv.wait(lock, [&] { return !msg_queue->empty(); });
        auto m = msg_queue->front();
        msg_queue->pop();
        lock.unlock();

        cout << m.id() << ": " << m.type() << endl;
    }
}

int realSendTextMsg(string msg, string receiver, string aters)
{
    cout << "To[" << receiver << "]: " << msg << endl;
    return 0;
}

int realSendImageMsg(string path, string receiver)
{
    cout << "To[" << receiver << "]: " << path << endl;
    return 0;
}

bool realGetMsgTypes(MsgTypes *types)
{
    const map<int32_t, string> tmp = { { 0x01, "文字" },
                                       { 0x03, "图片" },
                                       { 0x22, "语音" },
                                       { 0x25, "好友确认" },
                                       { 0x28, "POSSIBLEFRIEND_MSG" },
                                       { 0x2A, "名片" },
                                       { 0x2B, "视频" },
                                       { 0x2F, "石头剪刀布 | 表情图片" },
                                       { 0x30, "位置" },
                                       { 0x31, "共享实时位置、文件、转账、链接" },
                                       { 0x32, "VOIPMSG" },
                                       { 0x33, "微信初始化" },
                                       { 0x34, "VOIPNOTIFY" },
                                       { 0x35, "VOIPINVITE" },
                                       { 0x3E, "小视频" },
                                       { 0x270F, "SYSNOTICE" },
                                       { 0x2710, "红包、系统消息" },
                                       { 0x2712, "撤回消息" } };
    cout << "MsgTypes: " << tmp.size() << endl;

    MsgTypes mt;
    for (auto &[k, v] : tmp) { // C++17
        (*mt.mutable_types())[k] = v;
    }

    *types = move(mt);

    return true;
}

bool realGetContacts(Contacts *contacts)
{
    cout << "Contacts: " << 10 << endl;
    Contacts cnts;
    for (int i = 0; i < 10; i++) {
        string i_string = to_string(i);
        Contact *cnt    = cnts.add_contacts();
        cnt->set_wxid("wxid_" + i_string);
        cnt->set_code("code_" + i_string);
        cnt->set_name("name_" + i_string);
        cnt->set_country("country_" + i_string);
        cnt->set_province("province_" + i_string);
        cnt->set_city("city_" + i_string);
        cnt->set_gender("gender_" + i_string);
    }
    *contacts = move(cnts);

    return true;
}

bool realGetDbNames(DbNames *names)
{
    cout << "Dbs: " << 10 << endl;
    DbNames dbs;
    for (int i = 0; i < 10; i++) {
        string i_string = to_string(i);
        auto *name      = dbs.add_names();
        name->assign("db_" + i_string);
    }
    *names = move(dbs);

    return true;
}

bool realGetDbTables(const string db, DbTables *tables)
{
    cout << "Tables of " << db << ": " << 10 << endl;
    DbTables tbls;
    for (int i = 0; i < 10; i++) {
        string i_string = to_string(i);
        DbTable *tbl    = tbls.add_tables();
        tbl->set_name("table_" + i_string);
        tbl->set_sql("sql_" + i_string);
    }
    *tables = move(tbls);

    return true;
}

void RunServer()
{
    string server_address("localhost:50051");
    DemoImpl service(gMsgQueue, gMutex, gCv);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 2000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 3000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.RegisterService(&service);
    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << server_address << endl;
    server->Wait();
}

int main(int argc, char **argv)
{
    thread t1(ProduceMsg, &gMsgQueue); // 模拟生产消息
    // thread t2(ConsumeMsg, &gMsgQueue); // 验证消费消息

    RunServer();

    return 0;
}