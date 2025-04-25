#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

struct PriceSubmission {
    std::string oracleId;
    double price;
};

class PoPOracle {
    unsigned requiredSubmissions;
    std::map<std::string,double> submissions;

public:
    PoPOracle(unsigned k) : requiredSubmissions(k) {}

    void submit(const PriceSubmission& ps) {
        submissions[ps.oracleId] = ps.price;
        if (submissions.size() >= requiredSubmissions) {
            aggregate();
            submissions.clear();
        }
    }

    double getMedian() const {
        std::vector<double> prices;
        for (auto &p : submissions)
            prices.push_back(p.second);
        std::sort(prices.begin(), prices.end());
        return prices[prices.size()/2];
    }

private:
    void aggregate() {
        double m = getMedian();
        std::cout << "PoPOracle aggregated median = " << m << "\n";
    }
};
