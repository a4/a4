
#include <iomanip>
#include <iostream>
#include <string.h>

#include "a4/axis.h"
#include "a4/h2.h"

#include "pb/Histograms.pb.h"

using namespace std;

H2::H2(const uint32_t &xbins, const double &xmin, const double &xmax, const uint32_t &ybins, const double &ymin, const double &ymax):
    _x_axis(xbins, xmin, xmax),
    _y_axis(ybins, ymin, ymax),
    _entries(0)
{
    const uint32_t total_bins = (xbins + 2)*(ybins + 2);
    _data.reset(new double[total_bins]());
}

H2::H2(const H2 & h): 
    _x_axis(h._x_axis),
    _y_axis(h._y_axis),
    _entries(h._entries)
{
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    _data.reset(new double[total_bins]);
    memcpy(_data.get(), h._data.get(), total_bins * sizeof(double));
}

H2::~H2()
{
}
MessagePtr H2::get_message() {
    boost::shared_ptr<a4pb::Histogram2> h2(new a4pb::Histogram2);
    h2->mutable_x()->set_bins(_x_axis.bins());
    h2->mutable_x()->set_min(_x_axis.min());
    h2->mutable_x()->set_max(_x_axis.max());
    h2->mutable_y()->set_bins(_y_axis.bins());
    h2->mutable_y()->set_min(_y_axis.min());
    h2->mutable_y()->set_max(_y_axis.max());
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    for(uint32_t i = 0; i < total_bins; i++) h2->add_data(_data[i]);
    h2->set_entries(_entries);
    return h2;
}

H2::H2(Message& m) : _x_axis(0,0,0), _y_axis(0,0,0) {
    a4pb::Histogram2 * msg = dynamic_cast<a4pb::Histogram2*>(&m);
    _x_axis = Axis(msg->x().bins(), msg->x().min(), msg->x().max());
    _y_axis = Axis(msg->y().bins(), msg->y().min(), msg->y().max());
    _entries = msg->entries();
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    _data.reset(new double[total_bins]);
    for (int i = 0; i < total_bins; i++) _data[i] = msg->data(i);
}

double H2::integral() const
{
    double integral = 0;
    const int skip = _x_axis.bins() + 2;
    for(uint32_t bin = 1, bins = bin + _x_axis.bins(); bins > bin; ++bin) 
        for(uint32_t ybin = 1, ybins = ybin + _y_axis.bins(); ybins > ybin; ++ybin)
            integral += *(_data.get() + ybin*skip + bin);

    return integral;
}

void H2::fill(const double &x, const double & y, const double &weight)
{
    if (!_data)
        return;

    const int skip = _x_axis.bins() + 2;
    *(_data.get() + _x_axis.find_bin(x) + _y_axis.find_bin(y)*skip) += weight;
    ++_entries;
}

void H2::print(std::ostream &out) const
{
    if (!_data)
    {
        cerr << "Histogram is not initialized" << endl;
        return;
    }

    out << "X Axis: " << _x_axis << endl;
    out << "Y Axis: " << _y_axis << endl;
    out << "Entries: " << entries() << " Integral: " << integral() << endl;

    const int skip = _x_axis.bins() + 2;
    for(uint32_t bin = 1, bins = bin + _x_axis.bins(); bins > bin; ++bin) 
        for(uint32_t ybin = 1, ybins = ybin + _y_axis.bins(); ybins > ybin; ++ybin)
            out << "[" << setw(3) << bin << "]: " << setiosflags(ios::fixed) << setprecision(3) << *(_data.get() + ybin*skip + bin) << endl;
}

void H2::add(const H2 &source)
{
    const uint32_t total_bins = (_x_axis.bins() + 2)*(_y_axis.bins() + 2);
    for(uint32_t bin = 0, bins = total_bins; bins > bin; ++bin)
        *(_data.get() + bin) += *(source._data.get() + bin);
    _entries += source._entries;
}

// Helpers
//
ostream &operator<<(ostream &out, const H2 &hist)
{
    hist.print(out);

    return out;
}
