#include <catch2/catch.hpp>
#include <deserialize_json.h>
// #include <imgui.h>
#include <vector>

TEST_CASE("Default Scrolling Buffer", "[ScrollingBuffer]") {
    ScrollingBuffer scrollingBuffer;
    REQUIRE(scrollingBuffer.maxSize == 200000);
}
/*TEST_CASE("Scrolling Buffer with more points", "[ScrollingBuffer]") {
    ScrollingBuffer scrollingBuffer(2);
    scrollingBuffer.addPoint(1, 2);
    scrollingBuffer.addPoint(3, 4);
    scrollingBuffer.addPoint(5, 6);

    ImVector<DataPoint> testData;
    DataPoint           d1(3, 4); // wie deklariert man Datenpunkte
    DataPoint           d2(5, 6);
    testData.push_back(d1);
    testData.push_back(d2);

    REQUIRE(scrollingBuffer.maxSize == 2);
    REQUIRE(scrollingBuffer.data == testData); // wie vergleicht man den Inhalt
}*/
TEST_CASE("Empty Scrolling Buffer", "[ScrollingBuffer]") {
    ScrollingBuffer scrollingBuffer(0);
    scrollingBuffer.addPoint(1, 2);
    scrollingBuffer.addPoint(3, 4);

    REQUIRE(scrollingBuffer.maxSize == 0);
}
TEST_CASE("Deleted Scrolling Buffer", "[ScrollingBuffer]") {
    ScrollingBuffer scrollingBuffer(2);
    scrollingBuffer.addPoint(1, 2);
    scrollingBuffer.addPoint(3, 4);
    scrollingBuffer.erase();

    REQUIRE(scrollingBuffer.maxSize == 0);
}
