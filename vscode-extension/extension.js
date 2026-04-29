const vscode = require('vscode');
const path = require('path');

function activate(context) {
    console.log('Kore extension activated');

    let disposable = vscode.commands.registerCommand('kore.runFile', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active editor');
            return;
        }

        const document = editor.document;
        if (document.languageId !== 'kore') {
            vscode.window.showErrorMessage('Not a Kore file');
            return;
        }

        const filePath = document.fileName;
        const fileDir = path.dirname(filePath);
        const fileName = path.basename(filePath);

        const terminal = vscode.window.createTerminal({
            name: 'Kore',
            hideFromUser: false
        });
        terminal.show();
        
        terminal.sendText(`cd "${fileDir}"`);
        terminal.sendText(`kore "${fileName}"`);
    });

    context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};