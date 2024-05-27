#include <iostream>
#include <map>
#include <vector>

int main() {
    std::map<int, std::vector<int>> rowOfVectors;

    // Insert vectors with sample rates
    rowOfVectors[44100] = {1, 2, 3};
    rowOfVectors[48000] = {4, 5, 6};
    rowOfVectors[96000] = {7, 8, 9};

    // Access vectors by sample rate
    std::vector<int>& vector44100 = rowOfVectors[44100];
    std::vector<int>& vector48000 = rowOfVectors[48000];
    std::vector<int>& vector96000 = rowOfVectors[96000];

    // Print the vectors
    for (const auto& pair : rowOfVectors) {
        int sampleRate = pair.first;
        const std::vector<int>& vector = pair.second;

        std::cout << "Sample Rate: " << sampleRate << ", Vector: ";
        for (auto it = vector.cbegin(); it != vector.cend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <map>
#include <vector>

int main()
{
    std::map<int, std::pair<int, std::vector<double>>> myMap;

    // Insert values into the map
    myMap[1] = std::make_pair(10, std::vector<double>{1.1, 2.2, 3.3});
    myMap[2] = std::make_pair(20, std::vector<double>{4.4, 5.5, 6.6});
    myMap[3] = std::make_pair(30, std::vector<double>{7.7, 8.8, 9.9});

    // Access values from the map
    std::pair<int, std::vector<double>>& pair1 = myMap[1];
    std::pair<int, std::vector<double>>& pair2 = myMap[2];
    std::pair<int, std::vector<double>>& pair3 = myMap[3];

    // Print the values
    for (const auto& pair : myMap)
    {
        int index = pair.first;
        int intValue = pair.second.first;
        const std::vector<double>& doubleVector = pair.second.second;

        std::cout << "Index: " << index << ", Int Value: " << intValue << ", Double Vector: ";
        for (auto it = doubleVector.cbegin(); it != doubleVector.cend(); ++it)
        {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>
#include <vector>

int main() {
    std::map<int, std::tuple<int, int, std::vector<double>, std::vector<double>>> myMap;

    // Insert values into the map
    myMap[1] = std::make_tuple(10, 20, std::vector<double>{1.1, 2.2, 3.3}, std::vector<double>{4.4, 5.5, 6.6});
    myMap[2] = std::make_tuple(30, 40, std::vector<double>{7.7, 8.8, 9.9}, std::vector<double>{10.1, 11.2, 12.3});
    myMap[3] = std::make_tuple(50, 60, std::vector<double>{13.4, 14.5, 15.6}, std::vector<double>{16.7, 17.8, 18.9});

    // Access values from the map
    std::tuple<int, int, std::vector<double>, std::vector<double>>& tuple1 = myMap[1];
    std::tuple<int, int, std::vector<double>, std::vector<double>>& tuple2 = myMap[2];
    std::tuple<int, int, std::vector<double>, std::vector<double>>& tuple3 = myMap[3];

    // Print the values
    for (const auto& pair : myMap) {
        int index = pair.first;
        int intValue1 = std::get<0>(pair.second);
        int intValue2 = std::get<1>(pair.second);
        const std::vector<double>& doubleVector1 = std::get<2>(pair.second);
        const std::vector<double>& doubleVector2 = std::get<3>(pair.second);

        std::cout << "Index: " << index << ", Int Value 1: " << intValue1 << ", Int Value 2: " << intValue2 << ", Double Vector 1: ";
        for (auto it = doubleVector1.cbegin(); it != doubleVector1.cend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << ", Double Vector 2: ";
        for (auto it = doubleVector2.cbegin(); it != doubleVector2.cend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <map>
#include <vector>

int main()
{
    std::map<int, std::tuple<int, int, std::vector<int>::const_iterator, std::vector<int>::const_iterator>> myMap;

    // Insert values into the map
    std::vector<int> myVec1 = { 1, 2, 3 };
    std::vector<int> myVec2 = { 4, 5, 6 };
    std::vector<int> myVec3 = { 7, 8, 9 };

    myMap[1] = std::make_tuple(10, 20, myVec1.cbegin(), myVec2.cbegin());
    myMap[2] = std::make_tuple(30, 40, myVec2.cbegin(), myVec3.cbegin());
    myMap[3] = std::make_tuple(50, 60, myVec3.cbegin(), myVec1.cbegin());

    // Access values from the map
    std::tuple<int, int, std::vector<int>::const_iterator, std::vector<int>::const_iterator>& tuple1 = myMap[1];
    std::tuple<int, int, std::vector<int>::const_iterator, std::vector<int>::const_iterator>& tuple2 = myMap[2];
    std::tuple<int, int, std::vector<int>::const_iterator, std::vector<int>::const_iterator>& tuple3 = myMap[3];

    // Print the values
    for (const auto& pair : myMap)
    {
        int index = pair.first;
        int intValue1 = std::get<0>(pair.second);
        int intValue2 = std::get<1>(pair.second);
        std::vector<int>::const_iterator it1 = std::get<2>(pair.second);
        std::vector<int>::const_iterator it2 = std::get<3>(pair.second);

        std::cout << "Index: " << index << ", Int Value 1: " << intValue1 << ", Int Value 2: " << intValue2 << ", Vector Iterator 1: ";
        while (it1 != myVec1.cend())
        {
            std::cout << *it1 << " ";
            ++it1;
        }
        std::cout << ", Vector Iterator 2: ";
        while (it2 != myVec2.cend())
        {
            std::cout << *it2 << " ";
            ++it2;
        }
        std::cout << std::endl;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unordered_map>
#include <vector>

int main()
{
    std::unordered_map<int, std::vector<int>> rowOfVectors;

    // Insert vectors with sample rates
    rowOfVectors[44100] = { 1, 2, 3 };
    rowOfVectors[48000] = { 4, 5, 6 };
    rowOfVectors[96000] = { 7, 8, 9 };

    // Access vectors by sample rate
    std::vector<int>& vector44100 = rowOfVectors[44100];
    std::vector<int>& vector48000 = rowOfVectors[48000];
    std::vector<int>& vector96000 = rowOfVectors[96000];

    // Print the vectors
    for (const auto& pair : rowOfVectors)
    {
        int sampleRate = pair.first;
        const std::vector<int>& vector = pair.second;

        std::cout << "Sample Rate: " << sampleRate << ", Vector: ";
        for (auto it = vector.cbegin(); it != vector.cend(); ++it)
        {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
