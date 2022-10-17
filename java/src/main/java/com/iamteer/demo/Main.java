package com.iamteer.demo;

import java.util.List;
import java.util.Map;
import java.util.Iterator;

public class Main {
    public static void main(String[] args) throws Exception {
        String hostPort = "localhost:10086";
        Client client = new Client(hostPort);
        System.out.println("Connecting to " + hostPort);
        int status = client.IsLogin();
        System.out.println(status);

        String wxid = client.GetSelfWxid();
        System.out.println(wxid);

        client.EnableRecvMsg(); // Receive Message

        Map<Integer, String> msgTypes = client.GetMsgTypes();
        Iterator<Map.Entry<Integer, String>> iterTypes = msgTypes.entrySet().iterator();
        while (iterTypes.hasNext()) {
            Map.Entry<Integer, String> entry = iterTypes.next();
            System.out.println(entry.getKey() + ": " + entry.getValue());
        }

        List<DemoOuterClass.Contact> contacts = client.GetContacts();
        Iterator<DemoOuterClass.Contact> iterContacts = contacts.iterator();
        while (iterContacts.hasNext()) {
            DemoOuterClass.Contact contact = iterContacts.next();
            System.out.println(contact);
        }

        List<String> dbs = client.GetDbs();
        Iterator<String> iterDbs = dbs.iterator();
        while (iterDbs.hasNext()) {
            String db = iterDbs.next();
            System.out.println(db);
        }

        List<DemoOuterClass.DbTable> tables = client.GetTables("db");
        Iterator<DemoOuterClass.DbTable> iterTables = tables.iterator();
        while (iterTables.hasNext()) {
            DemoOuterClass.DbTable table = iterTables.next();
            System.out.println(table);
        }

        List<DemoOuterClass.DbRow> rows = client.QuerySql("db", "SELECT * FROM table_name LIMIT 1;");
        Iterator<DemoOuterClass.DbRow> iterRows = rows.iterator();
        while (iterRows.hasNext()) {
            DemoOuterClass.DbRow row = iterRows.next();
            System.out.println(row);
        }

        status = client.AcceptNewFriend("v3", "v4");
        System.out.println(status);

        Thread.sleep(5000);
        client.DisableRecvMsg();
    }

}