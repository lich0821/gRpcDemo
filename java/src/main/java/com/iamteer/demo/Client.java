package com.iamteer.demo;

import java.util.List;
import java.util.Map;

import io.grpc.ManagedChannel;
import io.grpc.stub.StreamObserver;
import io.grpc.ManagedChannelBuilder;

public class Client {
    Client(String hostPort) {
        this.connect(hostPort);
    }

    private void connect(String hostPort) {
        ManagedChannel managedChannel = ManagedChannelBuilder.forTarget(hostPort)
                .usePlaintext()
                .build();
        this.demoBlokingStub = DemoGrpc.newBlockingStub(managedChannel);
        this.demoStub = DemoGrpc.newStub(managedChannel);
    }

    public int IsLogin() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        DemoOuterClass.Response response = this.demoBlokingStub.rpcIsLogin(empty);
        return response.getStatus();
    }

    public String GetSelfWxid() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        DemoOuterClass.String rsp = this.demoBlokingStub.rpcGetSelfWxid(empty);
        return rsp.getStr();
    }

    public void EnableRecvMsg() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        this.demoStub.rpcEnableRecvMsg(empty, new StreamObserver<DemoOuterClass.WxMsg>() {
            @Override
            public void onNext(DemoOuterClass.WxMsg value) {
                System.out.printf("New Message:\n%s", value);
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("EnableRecvMsg Error");
            }

            @Override
            public void onCompleted() {
                System.out.println("EnableRecvMsg Complete");
            }
        });
    }

    public int DisableRecvMsg() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        DemoOuterClass.Response response = this.demoBlokingStub.rpcDisableRecvMsg(empty);
        return response.getStatus();
    }

    public int SendText(String msg, String receiver, String aters) {
        DemoOuterClass.TextMsg textMsg = DemoOuterClass.TextMsg.newBuilder().setMsg(msg).setReceiver(receiver)
                .setAters(aters).build();
        DemoOuterClass.Response response = this.demoBlokingStub.rpcSendTextMsg(textMsg);
        return response.getStatus();
    }

    public int SendImage(String path, String receiver) {
        DemoOuterClass.ImageMsg imageMsg = DemoOuterClass.ImageMsg.newBuilder().setPath(path).setReceiver(receiver)
                .build();
        DemoOuterClass.Response response = this.demoBlokingStub.rpcSendImageMsg(imageMsg);
        return response.getStatus();
    }

    public Map<Integer, String> GetMsgTypes() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        DemoOuterClass.MsgTypes msgTypes = this.demoBlokingStub.rpcGetMsgTypes(empty);
        return msgTypes.getTypesMap();
    }

    public List<DemoOuterClass.Contact> GetContacts() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        DemoOuterClass.Contacts contacts = this.demoBlokingStub.rpcGetContacts(empty);
        return contacts.getContactsList();
    }

    public List<String> GetDbs() {
        DemoOuterClass.Empty empty = DemoOuterClass.Empty.newBuilder().build();
        DemoOuterClass.DbNames dbs = this.demoBlokingStub.rpcGetDbNames(empty);
        return dbs.getNamesList();
    }

    public List<DemoOuterClass.DbTable> GetTables(String db) {
        DemoOuterClass.String str = DemoOuterClass.String.newBuilder().setStr(db).build();
        DemoOuterClass.DbTables tables = this.demoBlokingStub.rpcGetDbTables(str);
        return tables.getTablesList();
    }

    public List<DemoOuterClass.DbRow> QuerySql(String db, String sql) {
        DemoOuterClass.DbQuery query = DemoOuterClass.DbQuery.newBuilder().setDb(db).setSql(sql).build();
        DemoOuterClass.DbRows rows = this.demoBlokingStub.rpcExecDbQuery(query);
        return rows.getRowsList();
    }

    public int AcceptNewFriend(String v3, String v4) {
        DemoOuterClass.Verification v = DemoOuterClass.Verification.newBuilder().setV3(v3).setV4(v4).build();
        DemoOuterClass.Response response = this.demoBlokingStub.rpcAcceptNewFriend(v);
        return response.getStatus();
    }

    private DemoGrpc.DemoBlockingStub demoBlokingStub;
    private DemoGrpc.DemoStub demoStub;
}
