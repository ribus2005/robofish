#include "candle.h"
#include <gtest/gtest.h>

TEST(growing,is_red){
  Candle riba(300,500,400,600);
  EXPECT_EQ(riba.is_red(),0);  
}

TEST(neutral,is_red){
  Candle riba(0,0,0,0);
  EXPECT_EQ(riba.is_red(),0);
}

TEST(falling,is_red){
  Candle riba(500,400,200,100);
  EXPECT_EQ(riba.is_red(),1);
}