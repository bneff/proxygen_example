/*
 *  Copyright (c) 2015, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "DemoHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include <thread>

#include <folly/io/async/EventBaseManager.h>

using namespace proxygen;

namespace DemoService {

DemoHandler::DemoHandler() {
}

void DemoHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
}

void DemoHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (body_) {
    body_->prependChain(std::move(body));
  } else {
    body_ = std::move(body);
  }
}

void DemoHandler::onEOM() noexcept {

  folly::EventBase* evb = folly::EventBaseManager::get()->getExistingEventBase();
  std::thread t([&, evb]() {
  if( body_ )
  {
      printf("BODY: %.*s\n", (int)body_->length(), body_->data() );
  }
  else
  {
      body_ = folly::IOBuf::create(1024);
      std::string msg("Hello World!!!!!");
      memcpy(body_->writableData(), msg.data(), msg.size());
      body_->append(msg.size());
  }
  //sleep(10);

  evb->runInEventBaseThread([this]() {
  ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Custom", "1")
    .body( std::move(body_) )
    .sendWithEOM();
  });

  });
  t.detach();
}

void DemoHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
  // handler doesn't support upgrades
}

void DemoHandler::requestComplete() noexcept {
  printf("COMPLETE\n");
  delete this;
}

void DemoHandler::onError(ProxygenError err) noexcept {
  printf("ERROR\n");
  delete this;
}

}
