#include <array>
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <memory>
#include <sstream>
using namespace std;
class NaiveBayes
{
private:
	size_t alpha;
public:
	using EachTokenCount = vector<size_t>;
private:
	size_t totalTrain;
	// total number of training samples

	vector<size_t> labelCounts;
	vector<size_t> labelWordCounts;
	// counts of labels and counts of words in labels

	vector<EachTokenCount> wordCount;
	// count for each word for each label

	size_t vocabSize;
	// size of word vocab

	template <typename T=size_t>
	static void tryExpandIncVector(vector<T>& vec, size_t idx)
	{
		if (idx >= vec.size())
		{
			vec.resize(idx + 1, 0);
		}
		++vec[idx];
	}

	static void unify(vector<EachTokenCount>& vec)
	{
		size_t max = 0;
		for (const EachTokenCount& count : vec)
		{
			if (count.size() > max)
				max = count.size();
		}
		for (EachTokenCount& count : vec)
		{
			count.resize(max);
		}
	}

	static double getProb(size_t count, size_t total, size_t priorM1, size_t vocabSize)
	{
		return (double)(count + priorM1) / (total + priorM1 * vocabSize);
	}

public:
	NaiveBayes(size_t alpha=2)
		: alpha(alpha),
		totalTrain(0), vocabSize(0) {}

	void fit(const vector<vector<size_t>> &features, const vector<size_t> &labels)
	{
		assert(labels.size() == features.size());
		this->totalTrain += labels.size();
		for (size_t l : labels)
		{
			if (l >= this->labelCounts.size())
			{
				assert(l >= this->labelWordCounts.size());
				assert(l >= this->wordCount.size());
				this->labelCounts.resize(l + 1, 0);
				this->labelWordCounts.resize(l + 1, 0);
				this->wordCount.resize(l + 1, {});
			}
			// now all label vectors are extended
			++this->labelCounts[l];
		}

		for (size_t i = 0; i < labels.size(); i++)
		{
			this->labelWordCounts[labels[i]] += features[i].size();
			// count the number of paths and values

			for (size_t word : features[i])
			{
				tryExpandIncVector<>(this->wordCount[labels[i]], word);
			}
		}

		unify(this->wordCount);
		if (!this->wordCount.empty())
			this->vocabSize = this->wordCount.begin()->size();
		else
			assert(this->vocabSize == 0);
	}

	size_t predict(vector<size_t> &feature)
	{
		size_t alphaM1 = this->alpha - 1;
		size_t numLabel = this->labelCounts.size();
		vector<double> logProb(numLabel);
		for (size_t j = 0; j < numLabel; ++j)
		{
			logProb[j] = log((double)this->labelCounts[j] / this->totalTrain);
			for (size_t word : feature)
			{
				assert(this->wordCount[j].size() == this->vocabSize);
				if (word >= this->vocabSize)
					continue;
				logProb[j] += log(getProb(this->wordCount[j][word],
					this->labelWordCounts[j], alphaM1, vocabSize));
			}
		}
		return distance(logProb.begin(), max_element(logProb.begin(), logProb.end()));
	}

	void show()
	{
		size_t numLabel = this->labelCounts.size();
		cout << "Total number of labels: " << numLabel << endl;
		cout << "Total number of words: " << this->vocabSize << endl;
		cout << "Total number of training data: " << this->totalTrain << endl;
		cout << "Count for each label: " << endl;
		for (size_t l : this->labelCounts)
		{
			cout << l << ' ';
		}
		cout << endl;
		cout << "Count of words for each label: " << endl;
		for (size_t l : this->labelWordCounts)
		{
			cout << l << ' ';
		}
		cout << endl;
		cout << "Count of each word for each label: " << endl;
		for (const auto& eachLabel : this->wordCount)
		{
			for (size_t l : eachLabel)
			{
				cout << l << ' ';
			}
			cout << endl;
		}
		cout << "Probability distribution: " << endl;
		cout << "(probability of label, distribution of each word given label)" << endl;
		size_t alphaM1 = this->alpha - 1;
		for (size_t j = 0; j < numLabel; ++j)
		{
			cout << (double)this->labelCounts[j] / this->totalTrain << ", ";
			for (size_t word = 0; word < this->vocabSize; ++word)
			{
				cout << getProb(this->wordCount[j][word],
					this->labelWordCounts[j], alphaM1, vocabSize) << ' ';
			}
			cout << endl;
		}
	}

};


size_t menu()
{
	cout << "1. Create Model" << endl;
	cout << "2. Train Model" << endl;
	cout << "3. Show Model" << endl;
	cout << "4. Predict using Model" << endl;
	cout << "5. Remove Model" << endl;
	cout << "6. Exit" << endl;
	cout << "> ";
	size_t ret = 0;
	cin >> ret;
	return ret;
}

array<unique_ptr<NaiveBayes>, 0x10> models;

void createModel()
{
	auto empty = find(models.begin(), models.end(), nullptr);
	if (empty == models.end())
		return;
	cout << "Use custom alpha value[y/n]? ";
	char opt = 0;
	cin >> opt;
	if (opt == 'y' || opt == 'Y')
	{
		size_t alpha = 0;
		cout << "alpha = ";
		cin >> alpha;
		*empty = make_unique<NaiveBayes>(alpha);
	}
	else
	{
		*empty = make_unique<NaiveBayes>();
	}
	cout << "Model ID = " << empty - models.begin() << endl;
}

void trainModel()
{
	size_t idx = 0;
	cout << "Which model to train? ";
	cin >> idx;
	if (idx >= models.size() || models[idx] == nullptr)
		return;
	cout << "Please input training document features: (Use \"END\" to finish)" << endl;
	vector<vector<size_t>> features;
	string doc;
	while (true)
	{
		getline(cin, doc);
		if (doc == "END")
			break;
		if (doc.empty())
			continue;
		istringstream iss(move(doc));
		string word;
		vector<size_t> feature;
		while (getline(iss, word, ' '))
		{
			if (word.empty())
				continue;
			char* endptr = nullptr;
			size_t val = strtoul(word.c_str(), &endptr, 10);
			if (*endptr != '\x00')
			{
				cout << "Invalid word!" << endl;
				return;
			}
			feature.push_back(val);
		}
		features.push_back(move(feature));
	}

	cout << "Please input training document labels: " << endl;
	string labelsStr;
	getline(cin, labelsStr);
	istringstream iss(labelsStr);
	string label;
	vector<size_t> labels;
	while (getline(iss, label, ' '))
	{
		if (label.empty())
			continue;
		char* endptr = nullptr;
		size_t val = strtoul(label.c_str(), &endptr, 10);
		if (*endptr != '\x00')
		{
			cout << "Invalid label!" << endl;
			return;
		}
		labels.push_back(val);
	}
	if (labels.size() != features.size())
	{
		cout << "Unmatched training set!" << endl;
		cout << "Feature size = " << features.size() <<
			", Label size = " << labels.size() << endl;
		return;
	}

	models[idx]->fit(features, labels);
}

void predictModel()
{
	size_t idx = 0;
	cout << "Which model to use? ";
	cin >> idx;
	if (idx >= models.size() || models[idx] == nullptr)
		return;

	cout << "Please input testing document: " << endl;
	string doc;
	do
	{
		getline(cin, doc);
	}
	while (doc.empty());

	istringstream iss(doc);

	string word;
	vector<size_t> feature;
	while (getline(iss, word, ' '))
	{
		if (word.empty())
			continue;
		char* endptr = nullptr;
		size_t val = strtoul(word.c_str(), &endptr, 10);
		if (*endptr != '\x00')
		{
			cout << "Invalid word!" << endl;
			return;
		}
		feature.push_back(val);
	}

	cout << "Prediction result: " << models[idx]->predict(feature) << endl;
}

void showModel()
{
	size_t idx = 0;
	cout << "Which model to show? ";
	cin >> idx;
	if (idx >= models.size() || models[idx] == nullptr)
		return;
	models[idx]->show();
}

void removeModel()
{
	size_t idx = 0;
	cout << "Which model to remove? ";
	cin >> idx;
	if (idx >= models.size() || models[idx] == nullptr)
		return;
	models[idx] = nullptr;
}

int main(int argc, char const *argv[])
{
	setvbuf(stdout, nullptr, 2, 0);
	setvbuf(stderr, nullptr, 2, 0);
	setvbuf(stdin, nullptr, 2, 0);
	while (true)
	{
		switch (menu())
		{
			case 1:
			createModel();
			break;
			case 2:
			trainModel();
			break;
			case 3:
			showModel();
			break;
			case 4:
			predictModel();
			break;
			case 5:
			removeModel();
			break;
			default:
			return 0;
			break;
		}
	}
	return 0;
}

/*
Debug: g++ -Wall -g main.cpp -o bayes
Release: g++ -Wall -s -O0 main.cpp -o bayes
libc-2.31.so
*/