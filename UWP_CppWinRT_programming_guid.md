# UWP + C++/WinRT テンプレートでプログラミングする時の注意点

このメモは、`Learn-Composition` で実際につまずいた点を整理して、今後同じテンプレートで作業するときに迷わないための手順書です。

## まず押さえる前提

- `Learn-Composition` は `Windows Runtime Component (C++/WinRT)` のプロジェクトです。
- これは単体では起動できません。
- デバッグ実行するには、別途 UWP ホストアプリを用意して、そのアプリを Startup Project にする必要があります。
- コンポーネント側は「ロジックや WinRT 型を提供する側」、ホスト側は「起動と表示を担当する側」です。

## 今回の試行錯誤で分かったこと

### 1. コンポーネントだけでは Debug できない

最初に出たエラーはこれでした。

```text
The project 'Learn-Composition' cannot be started directly.
```

意味は単純で、`Learn-Composition` がアプリ本体ではないということです。
UWP で動かすなら、`Learn-Composition.App` のようなホストアプリを作って、そこからコンポーネントを使います。

### 2. `module.g.cpp` が必要だった

`main.cpp` がビルドできない原因の一つは、C++/WinRT の生成ファイルが不足していたことでした。

特に `module.g.cpp` をプロジェクトに含めないと、次のような未解決参照が出ます。

- `WINRT_CanUnloadNow`
- `WINRT_GetActivationFactory`

このテンプレートでは、生成ファイルを手で触るより、プロジェクト設定を正しくする方が大事です。

### 3. UWP で `init_apartment()` を呼ぶと落ちることがある

実行時に出ていた例外はこれでした。

```text
0x80010106 : Cannot change thread mode after it is set.
```

原因は、UWP の起動経路で既にスレッドの apartment が決まっているのに、`init_apartment(apartment_type::single_threaded)` を追加で呼んでいたことです。

結論として、UWP ホストでは `wWinMain()` で `init_apartment()` / `uninit_apartment()` を呼ばない方が安全です。

### 4. Composition の初期化順序が重要

`Compositor::CreateTargetForCurrentView()` は、`CoreWindow` の初期化が終わってから呼ぶ必要があります。

今回の構成では、`OnActivated()` の中で `CoreWindow::GetForCurrentThread().Activate()` を呼び、その後で Composition を初期化する形に直しました。

### 5. 古い `AppX` レイアウトが起動されることがある

Visual Studio でデバッグしているつもりでも、`AppX` 配置先に古い exe が残っていると、修正前のバイナリが起動されます。

今回もそれで混乱しました。

対策は次のどちらかです。

- 毎回 `Rebuild` してから起動する
- `AppX` レイアウトへ exe と manifest をコピーする後処理を入れる

## このテンプレートで作業するときの手順

1. まず「アプリ本体」ではなく「コンポーネント」として考える。
2. 実行用に UWP ホストアプリを用意する。
3. そのホストアプリを Startup Project に設定する。
4. ホスト側の `wWinMain()` は `CoreApplication::Run(make<AppViewSource>())` にする。
5. UWP では `init_apartment()` を呼ばない。
6. Composition 初期化は `OnActivated()` で行う。
7. UI や Composition の操作は `CoreWindow` が有効になってから行う。
8. ビルド後は、実際に起動される `AppX` の exe が最新か確認する。

## 実装時の実務メモ

- `main.cpp` は、コンポーネントのエントリではなく、ホストアプリの起動コードとして扱う。
- WinRT の初期化や Composition の失敗は、`winrt::hresult_error` を捕まえて `e.code()` と `e.message()` を出すと切り分けやすい。
- 生成ファイルは手で壊さない。必要なものが足りないときは、プロジェクト設定を直す。
- パッケージ系の不具合は、`AppxManifest.xml` と `AppX` 配置先の両方を見る。

## よくある症状と対処

### `cannot be started directly`

コンポーネントを直接起動しようとしている。
UWP ホストアプリを作る。

### `WINRT_CanUnloadNow` / `WINRT_GetActivationFactory`

`module.g.cpp` が入っていないか、生成物の参照が崩れている。

### `0x80010106`

`init_apartment()` を呼びすぎている可能性が高い。

### `hresult_error` だけ出る

例外の直前の WinRT 呼び出しが失敗している。
`OutputDebugString` でどの API まで進んだかを出す。

### 修正したのに古い挙動が残る

`AppX` 配置先が更新されていない。
`Rebuild` して、実際に起動される exe のタイムスタンプを確認する。

## 自分用の確認コマンド

```powershell
msbuild Learn-Composition.sln /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild Learn-Composition.App.vcxproj /t:Rebuild /p:Configuration=Debug /p:Platform=x64
```

## まとめ

このテンプレートで大事なのは、`Windows Runtime Component` と `UWP ホストアプリ` を分けて考えることです。
コンポーネントは再利用可能な WinRT コード、ホストは起動と表示です。

最初にここだけ覚えておけば、今回のような混乱はかなり減ります。
