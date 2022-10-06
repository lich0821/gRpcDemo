/*
RPC Client
*/

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

#include <memory>

#include <grpcpp/grpcpp.h>

#include "../proto/wcf.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using wcf::Contact;
using wcf::Contacts;
using wcf::DbField;
using wcf::DbNames;
using wcf::DbQuery;
using wcf::DbRow;
using wcf::DbRows;
using wcf::DbTable;
using wcf::DbTables;
using wcf::Empty;
using wcf::ImageMsg;
using wcf::MsgTypes;
using wcf::Response;
using wcf::String;
using wcf::TextMsg;
using wcf::Wcf;
using wcf::WxMsg;

class DemoClient
{
public:
    DemoClient(shared_ptr<Channel> channel)
        : stub_(Wcf::NewStub(channel))
    {
    }

    void SetMsgHandleCb(function<void(WxMsg &)> msg_handle_cb) { GetMessage(msg_handle_cb); }

    void GetMessage(function<void(WxMsg &)> msg_handle_cb)
    {
        class Reader : public grpc::ClientReadReactor<WxMsg>
        {
        public:
            Reader(Wcf::Stub *stub, function<void(WxMsg &)> msg_handle_cb)
                : msg_handle_cb_(msg_handle_cb)
            {
                stub->async()->GetMessage(&context_, &empty_, this);
                StartRead(&msg_);
                StartCall();
            }

            void OnReadDone(bool ok) override
            {
                if (ok) {
                    try {
                        msg_handle_cb_(msg_);
                    } catch (...) {
                        cout << "OnMsg wrong..." << endl;
                    }
                    StartRead(&msg_);
                }
            }

            void OnDone(const Status &s) override
            {
                unique_lock<mutex> l(mu_);
                status_ = s;
                done_   = true;
                cv_.notify_one();
            }

            Status Await()
            {
                unique_lock<mutex> l(mu_);
                cv_.wait(l, [this] { return done_; });
                return move(status_);
            }

        private:
            wcf::Empty empty_;
            wcf::WxMsg msg_;
            ClientContext context_;

            mutex mu_;
            Status status_;
            bool done_ = false;
            condition_variable cv_;

            function<void(WxMsg &)> msg_handle_cb_;
        };

        Reader reader(stub_.get(), msg_handle_cb);
        Status status = reader.Await();

        if (!status.ok()) {
            cout << "GetMessage rpc failed." << endl;
        }
    }

    int SendTextMsg(string msg, string receiver, string atusers)
    {
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        TextMsg t_msg;
        t_msg.set_msg(msg);
        t_msg.set_receiver(receiver);
        t_msg.set_aters(atusers);

        stub_->async()->SendTextMsg(&context, &t_msg, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "SendTextMsg rpc failed." << endl;
            rsp.set_status(-999); // TODO: Unify error code
        }

        return rsp.status();
    }

    int SendImageMsg(string path, string receiver)
    {
        Response rsp;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        ImageMsg i_msg;
        i_msg.set_path(path);
        i_msg.set_receiver(receiver);

        stub_->async()->SendImageMsg(&context, &i_msg, &rsp, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "SendImageMsg rpc failed." << endl;
            rsp.set_status(-999); // TODO: Unify error code
        }

        return rsp.status();
    }

    MsgTypes GetMsgTypes(void)
    {
        bool ret;
        Empty empty;
        MsgTypes mt;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->GetMsgTypes(&context, &empty, &mt, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetMsgTypes rpc failed." << endl;
        }

        return mt;
    }

    Contacts GetContacts(void)
    {
        bool ret;
        Empty empty;
        Contacts contacts;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->GetContacts(&context, &empty, &contacts, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetContacts rpc failed." << endl;
        }

        return contacts;
    }

    DbNames GetDbNames(void)
    {
        bool ret;
        Empty empty;
        DbNames names;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        stub_->async()->GetDbNames(&context, &empty, &names, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetDbNames rpc failed." << endl;
        }

        return names;
    }

    DbTables GetDbTables(string db)
    {
        bool ret;
        DbTables tables;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        String s_db;
        s_db.set_str(db);

        stub_->async()->GetDbTables(&context, &s_db, &tables, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "GetDbTables rpc failed." << endl;
        }

        return tables;
    }

    DbRows ExecDbQuery(string db, string sql)
    {
        bool ret;
        DbRows rows;
        ClientContext context;
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        Status status;

        DbQuery query;
        query.set_db(db);
        query.set_sql(sql);

        stub_->async()->ExecDbQuery(&context, &query, &rows, [&mu, &cv, &done, &status](Status s) {
            status = std::move(s);
            std::lock_guard<std::mutex> lock(mu);
            done = true;
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&done] { return done; });

        if (!status.ok()) {
            cout << "ExecDbQuery rpc failed." << endl;
        }

        return rows;
    }

private:
    unique_ptr<Wcf::Stub> stub_;
};

int OnMsg(WxMsg msg)
{
    cout << "Got Message: " << msg.is_self() << ", " << msg.is_group() << ", " << msg.type() << ", " << msg.id() << ", "
         << msg.xml() << ", " << msg.sender() << ", " << msg.roomid() << ", " << msg.content() << endl;
    return 0;
}

int main(int argc, char **argv)
{
    int ret;
    DemoClient wcf(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    ret = wcf.SendTextMsg("这是要发送的消息！", "wxid_fdjslajfdlajfldaj", "");
    cout << "SendTextMsg: " << ret << endl;

    ret = wcf.SendImageMsg("这是图片的路径！", "wxid_fdjslajfdlajfldaj");
    cout << "SendImageMsg: " << ret << endl;

    MsgTypes mts = wcf.GetMsgTypes();
    cout << "GetMsgTypes: " << mts.types_size() << endl;
    map<int32_t, string> m(mts.types().begin(), mts.types().end());
    for (auto &[k, v] : m) { // C++17
        cout << k << ": " << v << endl;
    }

    Contacts cnts = wcf.GetContacts();
    cout << "GetContacts: " << cnts.contacts().size() << endl;
    vector<Contact> vcnts(cnts.contacts().begin(), cnts.contacts().end());
    for (auto &c : vcnts) {
        cout << c.wxid() << "\t" << c.code() << "\t" << c.name() << "\t" << c.country() << "\t" << c.province() << "\t"
             << c.city() << "\t" << c.gender() << endl;
    }

    DbNames db = wcf.GetDbNames();
    cout << "GetDbNames: " << db.names().size() << endl;
    vector<string> dbs(db.names().begin(), db.names().end());
    for (auto &name : dbs) {
        cout << name << endl;
    }

    DbTables tbls = wcf.GetDbTables("db");
    cout << "GetDbTables: " << tbls.tables().size() << endl;
    vector<DbTable> vtbls(tbls.tables().begin(), tbls.tables().end());
    for (auto &tbl : vtbls) {
        cout << tbl.name() << "\n" << tbl.sql() << endl;
    }

    DbRows r = wcf.ExecDbQuery("MicroMsg.db", "SELECT * FROM Contact LIMIT 1;");
    cout << "ExecDbQuery: " << r.rows().size() << endl;
    vector<DbRow> vrows(r.rows().begin(), r.rows().end());
    for (auto &row : vrows) {
        vector<DbField> vfields(row.fields().begin(), row.fields().end());
        for (auto &field : vfields)
            cout << field.column() << "[" << field.type() << "]: " << field.content() << endl;
    }

    function<void(WxMsg &)> cb = OnMsg;
    wcf.SetMsgHandleCb(cb);

    return 0;
}
