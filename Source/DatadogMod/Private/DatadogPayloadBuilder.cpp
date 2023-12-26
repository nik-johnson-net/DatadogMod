#include "DatadogPayloadBuilder.h"

static FString HistogramKey(const FString& name, const TArray<FString>& tags) {
	FString key = FString(name);

	for (auto& tag : tags) {
		key = key + "|" + tag;
	}

	return key;
}


void DatadogPayloadBuilder::SetTimestamp(int64 ts)
{
	timestamp = ts;
}

void DatadogPayloadBuilder::SetInterval(int64 newInterval)
{
	interval = newInterval;
}

void DatadogPayloadBuilder::SetGlobalTags(TArray<FString>& newTags)
{
	tags = newTags;
}

void DatadogPayloadBuilder::AddGauge(FString name, TArray<FString>& ntags, double value, FString unit)
{
	AddMetric(EMetricType::Gauge, name, ntags, value, unit);
}

void DatadogPayloadBuilder::AddCounter(FString name, TArray<FString>& ntags, double value, FString unit)
{
	AddMetric(EMetricType::Count, name, ntags, value, unit);
}

void DatadogPayloadBuilder::AddRate(FString name, TArray<FString>& ntags, double value, FString unit)
{
	AddMetric(EMetricType::Rate, name, ntags, value, unit);
}

void DatadogPayloadBuilder::AddHistogram(FString name, TArray<FString>& ntags, double value, FString unit)
{
	auto key = HistogramKey(name, ntags);
	auto found = histograms.Find(key);
	if (found == nullptr) {
		DatadogHistogram histogram;
		histogram.name = name;
		histogram.tags = ntags;
		histogram.unit = unit;
		histogram.values.Add(value);
		histograms.Add(key, histogram);
	}
	else {
		found->values.Add(value);
	}
}

TArray<FDatadogTimeseries> DatadogPayloadBuilder::Build()
{
	// Building histograms must come before assigning global tags & metadata, since it creates new timeseries!
	BuildHistograms();

	// Assign global tags & metadata to all timeseries
	TArray<FDatadogTimeseries> builtTimeseries;

	for (auto& it : timeseries) {
		it.interval = interval;
		for (auto& tag : tags) {
			it.tags.Add(tag);
		}

		it.points[0].timestamp = timestamp;
	}

	return timeseries;
}

void DatadogPayloadBuilder::AddMetric(EMetricType type, const FString& name, TArray<FString>& ntags, double value, FString unit)
{
	FDatadogTimeseries newTimeseries;
	newTimeseries.metric = name;
	newTimeseries.tags = ntags;
	newTimeseries.unit = unit;
	newTimeseries.type = type;

	FTimeseriesPoint point;
	point.value = value;
	newTimeseries.points.Add(point);

	timeseries.Add(newTimeseries);
}

void DatadogPayloadBuilder::BuildHistograms()
{
	// Compute Histograms
	for (auto& histogram : histograms) {
		histogram.Value.values.Sort();

		// Calculate Sum
		double sum = 0;
		for (auto& it : histogram.Value.values) {
			sum += it;
		}

		AddMetric(EMetricType::Count, histogram.Value.name + ".count", histogram.Value.tags, histogram.Value.values.Num());
		AddMetric(EMetricType::Gauge, histogram.Value.name + ".sum", histogram.Value.tags, sum);
		AddMetric(EMetricType::Gauge, histogram.Value.name + ".max", histogram.Value.tags, histogram.Value.values.Last(), histogram.Value.unit);
		AddMetric(EMetricType::Gauge, histogram.Value.name + ".avg", histogram.Value.tags, sum / double(histogram.Value.values.Num()), histogram.Value.unit);
		AddMetric(EMetricType::Gauge, histogram.Value.name + ".median", histogram.Value.tags, histogram.Value.values[histogram.Value.values.Num() / 2], histogram.Value.unit);
		AddMetric(EMetricType::Gauge, histogram.Value.name + ".95percentile", histogram.Value.tags, histogram.Value.values[(histogram.Value.values.Num() * 95) / 100], histogram.Value.unit);
	}
}

FString DatadogHistogram::ToString()
{
	return HistogramKey(name, tags);
}
