#include "candle.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(growing,contains){
    std::cout << "\nrunning tests\n";
    Candle riba(300,500,400,600);
    EXPECT_EQ(riba.contains(500),1);
    EXPECT_EQ(riba.contains(400),1);
    EXPECT_EQ(riba.contains(600.1),0);
    EXPECT_EQ(riba.contains(399.9),0);
    
  }
  
  TEST(neutral,contains){
    Candle riba(0,0,0,0);
    EXPECT_EQ(riba.contains(0),1);
    EXPECT_EQ(riba.contains(-1),0);
  }
  
  TEST(falling,contains){
    Candle riba(500,400,200,100);
    EXPECT_EQ(riba.contains(400),1);
    EXPECT_EQ(riba.contains(200),1);
    EXPECT_EQ(riba.contains(300),1);
    EXPECT_EQ(riba.contains(501),0);
    EXPECT_EQ(riba.contains(99),0);
  }

  