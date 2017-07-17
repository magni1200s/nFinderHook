**nFinderHook**は、ハゲソフト製ニコニコ動画専用ブラウザであるnFinderを延命させるために作られたツールで、nFinderが抱える以下の2点の問題について対処します。

1. プレイヤーで動画を再生する際に「ヒストリークッキーまたは再生情報の取得に失敗しました。通信遅延によるタイムアウト・ログインが解除されている等の可能性があります」というエラーメッセージが表示されて動画の再生ができない
2. プレイヤーで動画を再生する際に投稿者コメントが取得、表示できない

ダウンロードは[こちら](https://github.com/magni1200s/nFinderHook/releases)からどうぞ。

動作環境は以下の通りです。
- nFinder バージョン2.1.0.2(32ビットのみ、64ビット化したバイナリは不可)
- Windows 7以上(ただし動作確認はWindows 10のみ)

nFinderHookは以下のモジュールで構成されます。

<ol>
<li>nFinderHook.dll
<ol>
<li>nFinderのプレイヤーが目的の動画をサーバーへリクエストする際に使用するWinsock2のsend()をフックし、HTTPリクエストヘッダーに含まれるCookieの値に「watch_flash=1」を追加することで、サーバーから新方式のHTML5形式ではなく従来のAdobe Flash形式の動画が送られてくるようにする
<li>nFinderのプレイヤーが投稿者コメントを取得するために送信するリクエストで使用するWinInetのHttpSendRequest()をフックし、HTTPリクエストヘッダーに含まれるContent-Typeの値を「text/xml」に変更し、正常なレスポンスが返ってくるようにする
</ol>
<li>nFinderLauncher.exe
  nFinderHook.dllをnFinder.exeに読み込ませるため(インジェクトするため)の起動用アプリケーション
</ol>

利用する際にはnFinderの実行ファイルであるnFinder.exeが存在するディレクトリにnFinderHook.dllとnFinderLauncher.exeの両方をコピーし、nFinderLauncher.exeを起動してください。一瞬コマンドプロンプト画面が表示された後に、nFinderが起動します。

なお利用しているサードパーティ製のライブラリの都合で32ビット版のnFinder.exeでしか利用できません。nFinderに同梱されているx64x86Conv.exeで64ビット化している場合はx64x86Conv.exeを使用して32ビットに戻してからお使いください。


このソフトウェアはDLLインジェクションのためにマイクロソフト社のDetours( https://www.microsoft.com/en-us/research/project/detours/ )を利用しています。ソースコードからビルドする際に同ライブラリ及びヘッダーファイルが必要となります。
また合わせてMalware Analyst's Cookbook( http://www.malwarecookbook.com/ )のサンプルコードも参考にさせていただきました。
