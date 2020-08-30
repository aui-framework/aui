#define BOOST_TEST_MODULE Tests
#include <boost/test/included/unit_test.hpp>
#include <AUI/Autumn/Autumn.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Data/ASqlBlueprint.h>
#include <AUI/Data/AMeta.h>
#include <AUI/Data/ASqlBuilder.h>
#include <AUI/Data/AModelMeta.h>
#include <AUI/Data/ASqlModel.h>
#include <AUI/Common/ASignal.h>
#include "AUI/Common/AString.h"
#include "AUI/Util/ARandom.h"
#include "AUI/Crypt/ARsa.h"
#include "AUI/Crypt/AX509.h"
#include "AUI/IO/FileInputStream.h"
#include "AUI/Crypt/AX509Store.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Reflect/AClass.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Util/Util.h"

using namespace boost::unit_test;



