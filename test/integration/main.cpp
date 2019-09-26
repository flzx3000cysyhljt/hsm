#include "hsm/hsm.h"

#include <boost/hana.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <future>

// States
struct S1 {
};
struct S2 {
};
struct S3 {
};
struct S4 {
};
struct S5 {
    constexpr auto on_entry(){
        return [](auto event){event.called->set_value();};    
    }
    constexpr auto on_exit(){
        return [](auto event){event.called->set_value();};
    }
};

// Events
struct e1 {
};
struct e2 {
};
struct e3 {
};
struct e4 {
    e4() : called(std::make_shared<std::promise<void>>()){}
    std::shared_ptr<std::promise<void>> called;    
};
struct e5 {
};
struct e6 {
    e6(const std::shared_ptr<std::promise<void>>& called) : called(called){}
    std::shared_ptr<std::promise<void>> called;    
};
struct e7 {
};
struct e8 {
};
struct e9 {
};
struct e10 {
    e10(const std::shared_ptr<std::promise<void>>& called) : called(called){}
    std::shared_ptr<std::promise<void>> called;    
};
struct e11 {
    e11(const std::shared_ptr<std::promise<void>>& called) : called(called){}
    std::shared_ptr<std::promise<void>> called;    
};
struct e12 {
};
struct e13 {
};


// Guards
const auto g1 = [](auto) { return true; };
const auto g2 = [](auto) { return false; };
const auto g3 = [](auto) { return true; };

// Actions
const auto a1 = [](auto event) {};
const auto a2 = [](auto event) {event.called->set_value();};

using namespace ::testing;
using namespace boost::hana;

struct SubSubState {
    constexpr auto make_transition_table()
    {
        // clang-format off        
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}));
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

struct SubState {
    constexpr auto make_transition_table()
    {
        // clang-format off        
        return hsm::transition_table(
            hsm::transition(S4 {}                              , hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S4 {}                              , hsm::event<e5> {}, g1, a1, S3 {}),
            hsm::transition(S4 {}                              , hsm::event<e6> {}, g1, a2, S4 {}),
            hsm::transition(S2 {}                              , hsm::event<e1> {}, g1, a1, SubSubState {}),
            hsm::transition(SubSubState {}                     , hsm::event<e2> {}, g1, a1, S4 {}),
            hsm::transition(hsm::Exit { SubSubState {}, S2 {} }, hsm::none {}     , g1, a1, S4 {}));
        // clang-format on
    }
    
    constexpr auto on_entry(){
        return [](auto event){event.called->set_value();};    
    }

    constexpr auto initial_state()
    {
        return S4 {};
    }
};

struct SubState2 {
    constexpr auto make_transition_table()
    {
        // clang-format off        
        return hsm::transition_table(
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}));
        // clang-format on
    }
    
    constexpr auto initial_state()
    {
        return S1 {};
    }
};


struct MainState {
    constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
            //              Source     , Event                    , Target
            hsm::transition(S1 {}, hsm::event<e1> {}, g1, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e2> {}, g1, a1, S3 {}),
            hsm::transition(S1 {}, hsm::event<e4> {}, g1, a1, SubState {}),
            hsm::transition(S1 {}, hsm::event<e5> {}, g1, a1, S3 {}),
            hsm::transition(S1 {}, hsm::event<e6> {}, g1, a2, S1 {}),
            hsm::transition(S1 {}, hsm::event<e7> {}, g2, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e8> {}, g3, a1, S2 {}),
            hsm::transition(S1 {}, hsm::event<e9> {}, g1, a1, hsm::Entry {SubState{}, S2{}}),
            hsm::transition(S1 {}, hsm::event<e10> {}, g1, a1, SubState {}),
            hsm::transition(S1 {}, hsm::event<e11> {}, g1, a1, S5 {}),
            hsm::transition(S1 {}, hsm::event<e13> {}, g1, a1, SubState2 {}),
            hsm::transition(S2 {}, hsm::event<e1> {}, g1, a1, S1 {}),
            hsm::transition(S2 {}, hsm::event<e2> {}, g1, a1, S1 {}),
            hsm::transition(S2 {}, hsm::event<e3> {}, g1, a1, S3 {}),
            hsm::transition(S3 {}, hsm::none {}, g1, a1, S1 {}),
            hsm::transition(S5 {}, hsm::event<e11> {}, g1, a1, S1 {}),
            hsm::transition(SubState {}, hsm::event<e2> {}, g1, a1, S1 {}),
            hsm::transition(S1{}, hsm::event<e12> {}, g1, a1, hsm::Direct{SubState{}, S4{}}),
            hsm::transition(hsm::Direct{SubState{}, S4{}}, hsm::event<e12> {}, g1, a1, hsm::Direct{SubState2{}, S1{}}));
        // clang-format on
    }

    constexpr auto initial_state()
    {
        return S1 {};
    }
};

class HsmTests : public Test {
    hsm::Sm<MainState> sm;
};

TEST_F(HsmTests, should_start_in_initial_state)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(HsmTests, should_start_in_root_state)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
}

TEST_F(HsmTests, should_process_event)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));

    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(S2 {}));
}

TEST_F(HsmTests, should_throw_on_unexpected_event)
{
    hsm::Sm<MainState> sm;
    EXPECT_THROW(sm.process_event(e3 {}), std::exception);
}

TEST_F(HsmTests, should_transit_into_SubState)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});

    ASSERT_TRUE(sm
    .is(SubState {}, S4 {}));
}

TEST_F(HsmTests, should_call_entry_action)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();

    hsm::Sm<MainState> sm;
    sm.process_event(e10 {entryActionCalled});

    ASSERT_EQ(std::future_status::ready, entryActionCalled->get_future().wait_for(std::chrono::seconds(1)));

}

TEST_F(HsmTests, should_call_entry_and_exit_action)
{
    auto entryActionCalled = std::make_shared<std::promise<void>>();    
    auto exitActionCalled = std::make_shared<std::promise<void>>();

    hsm::Sm<MainState> sm;
    sm.process_event(e11 {entryActionCalled});
    ASSERT_TRUE(sm.is(S5 {}));
    sm.process_event(e11 {exitActionCalled});

    ASSERT_EQ(std::future_status::ready, exitActionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(HsmTests, should_transit_into_SubSubState)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});

    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));
}

TEST_F(HsmTests, should_transit_in_SubState_with_unique_event)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e5 {});

    ASSERT_TRUE(sm.is(SubState {}, S3 {}));
}

TEST_F(HsmTests, should_exit_substate_on_event_in_parentstate)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(SubState {}, S4 {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(MainState {}, S1 {}));
}

TEST_F(HsmTests, should_entry_substate_on_pseudo_entry)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e9 {});
    ASSERT_TRUE(sm.is(SubState {}, S2 {}));
}

TEST_F(HsmTests, should_exit_subsubstate_on_event_in_parentstate)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));

    sm.process_event(e2 {});
    ASSERT_TRUE(sm.is(SubState {}, S4 {}));
}

TEST_F(HsmTests, should_exit_subsubstate_from_pseudo_exit)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e1 {});
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubSubState {}, S1 {}));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(SubState {}, S4 {}));
}

TEST_F(HsmTests, should_transit_with_anonymous_transition)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e5 {});
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(HsmTests, should_call_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    hsm::Sm<MainState> sm;
    sm.process_event(e6 {actionCalled});
    
    ASSERT_EQ(std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(HsmTests, should_call_substate_action)
{
    auto actionCalled = std::make_shared<std::promise<void>>();

    hsm::Sm<MainState> sm;
    sm.process_event(e4 {});
    sm.process_event(e6 {actionCalled});
    
    ASSERT_EQ(std::future_status::ready, actionCalled->get_future().wait_for(std::chrono::seconds(1)));
}

TEST_F(HsmTests, should_block_transition_guard)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e7 {});
    ASSERT_TRUE(sm.is(S1 {}));
}

TEST_F(HsmTests, should_not_block_transition_by_guard)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));
    sm.process_event(e8 {});
    ASSERT_TRUE(sm.is(S2 {}));
}

TEST_F(HsmTests, should_transit_directly_into_substate)
{
    hsm::Sm<MainState> sm;
    sm.process_event(e12 {});
    ASSERT_TRUE(sm.is(SubState{}, S4 {}));
}

TEST_F(HsmTests, should_transit_directly_between_substates)
{
    hsm::Sm<MainState> sm;
    
    sm.process_event(e4 {});
    ASSERT_TRUE(sm.is(SubState{}, S4 {}));
    sm.process_event(e12 {});    
    ASSERT_TRUE(sm.is(SubState2{}, S1 {}));
}

TEST_F(HsmTests, should_process_alot_event)
{
    hsm::Sm<MainState> sm;
    ASSERT_TRUE(sm.is(S1 {}));

    for (int i = 0; i < 1000000; i++) {
        sm.process_event(e1 {});
    }
}