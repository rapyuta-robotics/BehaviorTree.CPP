#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/decorators/retry_node.h"

using BT::NodeStatus;

class FinishAfterN : public BT::SyncActionNode
{
public:
  FinishAfterN(const std::string& name, int N):
    BT::SyncActionNode(name, {}), counter_(N)
  {
  }

  NodeStatus tick() override
  {
    if (counter_-- > 0)
    {
      std::string msg = return_value == NodeStatus::SUCCESS ? "SUCCESS" : "FAILURE";
      std::cout << name() << "; " << msg << std::endl;
      return return_value;
    }
    else
    {
      if (return_value == NodeStatus::SUCCESS)
      {
        std::cout << name() << "; FAILURE" << std::endl;
        return NodeStatus::FAILURE;
      }
      else
      {
        std::cout << name() << "; SUCCESS" << std::endl;
        return NodeStatus::SUCCESS;
      }
    }
  }

  NodeStatus return_value = NodeStatus::SUCCESS;
  private:
    int counter_ = 0;
};

/**
 * This test has a reactive sequence with a condition that return SUCCESS in the first tick and FAILURE in the second tick.
 * The other child in the reactive sequence is an action with a retry decorator.
 * The Action returns FAILURE in the first tick and SUCCESS in the second tick.
 * The retry decorator has a max_attempts of 2.
 *
 * In the first tick, the condition returns SUCCESS, thus the action is ticked, but it returns FAILURE.
 * Since the decorator is a retry, we expect it to return RUNNING.
 * In the second tick, the condition return FAILURE, the retry should be halted and the action is not ticked.
 * The result of the reactive sequence should be FAILURE.
*/
TEST(Retry, test)
{
  BT::ReactiveSequence root("root");
  BT::RetryNode retry("retry", 2);
  FinishAfterN condition_1("condition_1", 1);
  FinishAfterN action_1("action_1", 1);
  root.addChild(&condition_1);
  retry.setChild(&action_1);
  root.addChild(&retry);

  condition_1.return_value = BT::NodeStatus::SUCCESS;
  action_1.return_value = BT::NodeStatus::FAILURE;

  BT::NodeStatus status = root.executeTick();
  ASSERT_EQ(status, BT::NodeStatus::RUNNING);

  status = root.executeTick();
  ASSERT_EQ(status, BT::NodeStatus::FAILURE);
}


