#include "candle.h"
#include <gtest/gtest.h>

TEST(growing,is_green){
    Candle riba(300,500,400,600);
    EXPECT_EQ(riba.is_green(),1);  
  }
  
  TEST(neutral,is_green){
    Candle riba(0,0,0,0);
    EXPECT_EQ(riba.is_green(),0);
  }
  
  TEST(falling,is_green){
    Candle riba(500,400,200,100);
    EXPECT_EQ(riba.is_green(),0);
  }