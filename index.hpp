#pragma once

#include <winsock2.h>
#include <Eigen/Dense>
#include <future>
#include <mutex>
#include <chrono>
#include <pdh.h>
#include "CGNE.hpp"
#include "CGNR.hpp"
#include "matrix.hpp"
#include <windows.h>
#include "ImageGeneration.hpp"
#include "json.hpp"
#define BOOST_ASIO_DISABLE_IOCP
//libs baixadas e headers do autor
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <boost/json.hpp>
#include "threadPool.hpp"