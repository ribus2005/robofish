#include "candle.h"
#include <gtest/gtest.h>

TEST(growing,body_size){
    Candle riba(300,500,400,600);
    EXPECT_EQ(riba.body_size(),300);  
  }
  
  TEST(neutral,body_size){
    Candle riba(0,0,0,0);
    EXPECT_EQ(riba.body_size(),0);
  }
  
  TEST(falling,body_size){
    Candle riba(500,400,200,100);
    EXPECT_EQ(riba.body_size(),400);
  }