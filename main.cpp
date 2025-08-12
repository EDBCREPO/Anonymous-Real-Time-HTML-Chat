#include <nodepp/nodepp.h>
#include <express/http.h>

using namespace nodepp;

queue_t<express_http_t> clients;

void send_message_handler( queue_t<express_http_t>& clients, string_t message ){
    string_t flt  = regex::replace_all( message.slice(2), "[><]+", "" );
    string_t msg  = regex::format( "<div>${0}</div>", flt );
    string_t data = encoder::hex::get( msg.size() ) + "\r\n" + msg + "\r\n";
    auto n = clients.first(); while( n != nullptr ) { n->data.write( data ); n=n->next; }
}

express_tcp_t router() {
    auto app = express::http::add();
    
    app.ALL("/msg",[]( express_http_t cli ){
    
        cli.header( "Transfer-Encoding", "chunked" );
        cli.header( "Content-Type", "text/html" );
        cli.send(); cli.set_timeout(0); 
        
        console::log( "client connected" );
    
        string_t payload = R"(<!DOCTYPE html> <html lang="en">
            <head> <meta charset="UTF-8"> <title>basepage</title>   
                   <link rel="stylesheet" href="/uikit.css">
                   <meta name="viewport" content="width=device-width, initial-scale=1.0">
            </head> <body class="uk-background-dark uk-height-expand uk-padding-small uk-flex uk-flex-column"> 
        )"; 
    
        clients.push( cli ); auto ID = clients.last(); 
    
        cli.write( encoder::hex::get(payload.size()) + "\r\n" + payload + "\r\n" );
        
        cli.onDrain([=](){
            console::log( "client disconnected" );
            clients.erase( ID );
        });
    
        stream::pipe( cli );
    
    });

    app.ALL("/form",[]( express_http_t cli ){

        if( cli.headers["Content-Length"].empty() == false &&
            cli.headers["Content-Type"] == "text/plain" &&
            cli.method == "POST" 
        ) {
            auto len = string::to_ulong(cli.headers["Content-Length"]);
            send_message_handler( clients, cli.read(len) );
            cli.redirect("/api/form"); return;
        }  
        
        cli.sendFile( path::join("www","form.html") );
    
    });

    return app;
}

void onMain() {

    auto app = express::http::add();

    app.USE( "/api", router() );
    
    app.USE( express::http::file("www") );

    app.listen( "0.0.0.0", 8000, []( ... ){
        console::log( "http://localhost:8000" );
    });

}