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
4. ビルド
   　<CMake 使用しない場合>
     - gcc main.c mongoose.c -o server -lsqlite3 -lpthread
     <CMake 使用する場合>
     - server/build を作成
     - cd build でbuild フォルダへ移動
     - cmake -G Ninja ..　を実行
       コマンドの説明
       　cmake : CMake は、クロスプラットフォームのビルドシステム生成ツールです。ソースコードからビルドシステム（例: Makefile、Ninjaビルドファイルなど）を生成します。
        -G Ninja : -G オプションは、CMake にどのビルドシステムを生成するかを指定します。この場合、Ninja を指定しているため、Ninja ビルドシステム用のビルドファイルを生成します。Ninja は、高速で効率的なビルドツールです。
        .. : .. は、CMake にプロジェクトのルートディレクトリ（CMakeLists.txt が存在するディレクトリ）を指定しています。この場合、現在のディレクトリの1つ上のディレクトリを指定しています。
     - ninja
       コマンドの説明
       　ninja このコマンドでNinjaが生成されたビルドファイルを使用してプロジェクトをビルドする。
       　同時に、server/web, server/sample.db が server/build/にコピーされる。
     
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


＜運用方法＞
MY-MG-ANGULAR/server/build の中の　./server の実行ファイルには、 sample.db や web(Angularアプリ)は含まれていないので、databaseやangular アプリを変更したい場合は、sample.db や web を入れ替えて ./serverを再実行すれば良い。つまり、./server のリビルドは不要。

最終的に配布するのは例えばこんな構成：
release-package/
 ├─ server          ← CMakeでビルドした実行ファイル
 ├─ sample.db       ← SQLiteデータベース
 └─ web/            ← Angularビルド成果物 (browser/)
     ├─ index.html
     ├─ main-xxxx.js
     └─ styles-xxxx.css

利用者はこのフォルダをデバイスに置いて、
./server
を実行すれば良い。サーバーは sample.db と web をその場で読み取る。

