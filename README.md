Mongoose web server に　Angularアプリの配信　と　SQLiteDatabase のAPIの提供をさせた

./server を実行すればあとは、ブラウザーから http://localhost:8000/ にアクセスすれば良い。

なお、サーバー側　と　Angular側のビルド方法は以下の通り

＜本番環境構築＞
<Angular>
1. ng build --configuration production
2. cp -r dist/angular-app/browser/* ../server/web/


<server>
1. SQLite database の db file 作成： sqlite3 sample.db
2. 以下のコードを貼り付ける。これで sample.db が作成される

CREATE TABLE categories (
  id INTEGER PRIMARY KEY,
  name TEXT
);

CREATE TABLE products (
  id INTEGER PRIMARY KEY,
  name TEXT,
  price REAL,
  category_id INTEGER,
  FOREIGN KEY(category_id) REFERENCES categories(id)
);

INSERT INTO categories (name) VALUES ('Fruit'), ('Drink');
INSERT INTO products (name, price, category_id) VALUES
  ('Apple', 120, 1),
  ('Orange', 150, 1),
  ('Cola', 180, 2);
.exit

3. mongoose.c , mongoose.h をネットから取得して、 server/ に配置
4. gcc main.c mongoose.c -o server -lsqlite3 -lpthread
5. これで ./server でサーバーが起動する
6. http://localhost:8000/ にブラウザーからアクセス
7. これで ./server 単体(Mongoose server 単体)でブラウザーでコンテンツが見れる



＜開発環境＞
<Angular>
1. ng serve --proxy-config proxy.conf.json
2. http://localhost:4200/ にブラウザーからアクセス

<Server>
上記で作成した ./server を起動

これで、開発環境で動作する。Angularのコードを変更すると自動で再ビルドされて結果がブラウザーに反映される。
