#include "DatadogPayloadBuilder.h"

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

void DatadogPayloadBuilder::AddHistogram(FString name, TArray<FString>& ntags, TArray<double> values, FString unit)
{
	// TODO: This should all be calculated on Build(). Store value indexed by name, ntags instead.
	values.Sort();

	// Calculate Sum
	double sum = 0;
	for (auto& it : values) {
		sum += it;
	}

	AddMetric(EMetricType::Count, name + ".count", ntags, values.Num());
	AddMetric(EMetricType::Gauge, name + ".max", ntags, values.Last(), unit);
	AddMetric(EMetricType::Gauge, name + ".avg", ntags, sum / double(values.Num()), unit);
	AddMetric(EMetricType::Gauge, name + ".median", ntags, values[values.Num() / 2], unit);
	AddMetric(EMetricType::Gauge, name + ".95percentile", ntags, values[(values.Num() * 95) / 100], unit);
}

TArray<FDatadogTimeseries> DatadogPayloadBuilder::Build()
{
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
