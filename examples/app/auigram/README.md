# AUI Telegram Client (AUIgram)

<!-- aui:example app -->
Using AUI Framework from perspective of a Telegram client. This project shows how to solve typical problems of a UI
application (i.e., dependency management, data binding, styling, asynchronous data loading, packaging, deploying).

![AUI Telegram Client](https://github.com/aui-framework/telegram_client/blob/f985af77711be2b17b6aab11bfafb2a864800a1e/demo/demo.jpg?raw=true)

## Motivation/rationale

We wanted some kind of project/repository to be as an example/reference project of a real life application built
entirely with AUI.

Telegram Client is a great example:

- `tdlib`, which is basis of every Telegram client, uses C++
- almost everyone is familiar with Telegram
- there are many challenges for AUI, such as importing `tdlib` with [aui.boot] (spoiler: it does the job
  perfectly), asynchronous data loading, data binding, animations, assets, full support of Windows, macOS, Linux,
  Android, iOS.

## Source Code

This example is located outside AUI's source tree. Checkout its
[repository](https://github.com/aui-framework/telegram_client).

