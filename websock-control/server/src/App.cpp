#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"
#include "oatpp/network/server/Server.hpp"
#include <iostream>


// Sbus Service Instance
SBUS_CONTROLLER::Sbus_Service sbus_service;

void run() {

  /* Register Components in scope of run() method */
  AppComponent components;

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create MyController and add all of its endpoints to router */
  auto myController = std::make_shared<MyController>();
  myController->addEndpointsToRouter(router);

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, connectionHandler, "http");

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::server::Server server(connectionProvider, connectionHandler);

  /* Priny info about server port */
  OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

  /* Run server */
  server.run();

}

int main(int argc, const char * argv[]) {

  /* SBUS start */
  if(sbus_service.setup() == -1)
  {
    OATPP_LOGE("MyApp", "FATAL: sbus_service init failed !!");
    exit(-1);
  }

  oatpp::base::Environment::init();

  run();

  oatpp::base::Environment::destroy();

  return 0;
}