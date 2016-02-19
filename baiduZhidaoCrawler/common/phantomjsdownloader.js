var system = require('system');
if (system.args.length >= 2){
    var url = system.args[1];
    var page = require('webpage').create();
    page.customHeaders = {
        'Accept-Language': 'zh-CN,zh;q=0.8,en;q=0.6',
        'Connection': 'keep-alive',
        'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8'
    };
    page.settings.userAgent = 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.85 Safari/537.36';
    page.open(encodeURI(url),function(status){
        if (status != 'success'){
            console.log('Err, status=' + status);
            phantom.exit(1);
        }
        console.log(page.content);
        phantom.exit();
    });
}