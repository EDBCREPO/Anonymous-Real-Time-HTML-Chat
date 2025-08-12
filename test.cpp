#include <nodepp/nodepp.h>
#include <express/http.h>

using namespace nodepp;

queue_t<express_http_t> clients;

void onMain() {

    auto app = express::http::add();

    app.GET( [=]( express_http_t cli ){
    
        cli.header( "Transfer-Encoding", "chunked" );
        cli.header( "Content-Type", "text/html" );
        cli.send(); cli.set_timeout(0); 
        
        console::log( "client connected" );
    
        string_t payload = R"(<!DOCTYPE html> <html lang="en">
            <head> <meta charset="UTF-8"> <title>basepage</title>
                   <meta name="viewport" content="width=device-width, initial-scale=1.0">
            </head> <body>
        )"; 
    
        cli.write( encoder::hex::get(payload.size()) + "\r\n" + payload + "\r\n" );

        process::add( coroutine::add( COROUTINE(){
        coBegin

            for(;;){ do {

                auto payload = string_t( "hello world! <br>" );
                cli.write( encoder::hex::get(payload.size()) + "\r\n" + payload + "\r\n" );

            } while(0); coDelay(1000); }

        coFinish
        }));

    });

    app.listen( "0.0.0.0", 8000, []( ... ){
        console::log( "http://localhost:8000" );
    });

}