#include "candle.h"
#include <gtest/gtest.h>

TEST(growing,body_contains){
  Candle riba(300,500,400,600);
  EXPECT_EQ(riba.body_contains(500),1);
  EXPECT_EQ(riba.body_contains(400),1);
  EXPECT_EQ(riba.body_contains(600.1),0);
  EXPECT_EQ(riba.body_contains(599.9),1);
  
}

TEST(neutral,body_contains){
    Candle riba(0,0,0,0);
    EXPECT_EQ(riba.body_contains(0),1);
    EXPECT_EQ(riba.body_contains(-1),0);
  }
  
  TEST(falling,body_contains){
    Candle riba(500,400,200,100);
    EXPECT_EQ(riba.body_contains(400),1);
    EXPECT_EQ(riba.body_contains(200),1);
    EXPECT_EQ(riba.body_contains(300),1);
    EXPECT_EQ(riba.body_contains(501),0);
    EXPECT_EQ(riba.body_contains(99),0);
  }