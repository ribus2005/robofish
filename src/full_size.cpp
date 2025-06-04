 #include "candle.h"
#include <gtest/gtest.h>
  
  TEST(growing,full_size){
    Candle riba(300,500,400,600);
    EXPECT_EQ(riba.full_size(),100);  
  }
  
  TEST(neutral,full_size){
    Candle riba(0,0,0,0);
    EXPECT_EQ(riba.full_size(),0);
  }
  
  TEST(falling,full_size){
    Candle riba(500,400,200,100);
    EXPECT_EQ(riba.full_size(),200);
  }