/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "gtest/gtest.h"

#include "cybertron/blocker/blocker.h"
#include "cybertron/proto/unit_test.pb.h"

namespace apollo {
namespace cybertron {
namespace blocker {

using apollo::cybertron::proto::UnitTest;

TEST(BlockerTest, constructor) {
  BlockerAttr attr(10, "channel");
  Blocker<UnitTest> blocker(attr);
  EXPECT_EQ(blocker.capacity(), 10);
  EXPECT_EQ(blocker.channel_name(), "channel");

  blocker.set_capacity(20);
  EXPECT_EQ(blocker.capacity(), 20);
}

TEST(BlockerTest, publish) {
  BlockerAttr attr(10, "channel");
  Blocker<UnitTest> blocker(attr);

  auto msg1 = std::make_shared<UnitTest>();
  msg1->set_class_name("BlockerTest");
  msg1->set_case_name("publish_1");

  UnitTest msg2;
  msg2.set_class_name("BlockerTest");
  msg2.set_case_name("publish_2");

  EXPECT_TRUE(blocker.IsPublishedEmpty());
  blocker.Publish(msg1);
  blocker.Publish(msg2);
  EXPECT_FALSE(blocker.IsPublishedEmpty());

  EXPECT_TRUE(blocker.IsObservedEmpty());
  blocker.Observe();
  EXPECT_FALSE(blocker.IsObservedEmpty());

  auto& latest_observed_msg = blocker.GetLatestObserved();
  EXPECT_EQ(latest_observed_msg.class_name(), "BlockerTest");
  EXPECT_EQ(latest_observed_msg.case_name(), "publish_2");

  auto latest_observed_msg_ptr = blocker.GetLatestObservedPtr();
  EXPECT_EQ(latest_observed_msg_ptr->class_name(), "BlockerTest");
  EXPECT_EQ(latest_observed_msg_ptr->case_name(), "publish_2");

  auto latest_published_ptr = blocker.GetLatestPublishedPtr();
  EXPECT_EQ(latest_published_ptr->class_name(), "BlockerTest");
  EXPECT_EQ(latest_published_ptr->case_name(), "publish_2");

  blocker.ClearPublished();
  blocker.ClearObserved();
  EXPECT_TRUE(blocker.IsPublishedEmpty());
  EXPECT_TRUE(blocker.IsObservedEmpty());
}

TEST(BlockerTest, subscribe) {
  BlockerAttr attr(10, "channel");
  Blocker<UnitTest> blocker(attr);

  auto received_msg = std::make_shared<UnitTest>();
  bool res = blocker.Subscribe(
      "BlockerTest1", [&received_msg](const std::shared_ptr<UnitTest>& msg) {
        received_msg->CopyFrom(*msg);
      });

  EXPECT_TRUE(res);

  auto msg1 = std::make_shared<UnitTest>();
  msg1->set_class_name("BlockerTest");
  msg1->set_case_name("publish_1");

  blocker.Publish(msg1);

  EXPECT_EQ(received_msg->class_name(), msg1->class_name());
  EXPECT_EQ(received_msg->case_name(), msg1->case_name());

  res = blocker.Subscribe(
      "BlockerTest1", [&received_msg](const std::shared_ptr<UnitTest>& msg) {
        received_msg->CopyFrom(*msg);
      });

  EXPECT_FALSE(res);

  blocker.Reset();
  res = blocker.Subscribe(
      "BlockerTest1", [&received_msg](const std::shared_ptr<UnitTest>& msg) {
        received_msg->CopyFrom(*msg);
      });
  EXPECT_TRUE(res);
  blocker.Publish(msg1);

  EXPECT_EQ(received_msg->class_name(), msg1->class_name());
  EXPECT_EQ(received_msg->case_name(), msg1->case_name());

  res = blocker.Unsubscribe("BlockerTest1");
  EXPECT_TRUE(res);
  res = blocker.Unsubscribe("BlockerTest1");
  EXPECT_FALSE(res);

  UnitTest msg2;
  msg2.set_class_name("BlockerTest");
  msg2.set_case_name("publish_2");

  blocker.Publish(msg2);
  EXPECT_NE(received_msg->case_name(), msg2.case_name());
}

}  // namespace blocker
}  // namespace cybertron
}  // namespace apollo

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
