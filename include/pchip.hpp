#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>



class pchip_interpolation
{
    private:
        u_int num_points;
        std::vector<double> x_;
        std::vector<double> y_;
        std::vector<double> h_;
        std::vector<double> del_;
        std::vector<double> d_;

    public:

        pchip_interpolation(
            const std::vector<double>& x,
            const std::vector<double>& y
        ) : x_(x), y_(y)
        {

            if (x.size() != y.size())
                throw std::invalid_argument("Invalid Argument, X and Y must have the same size");

            if (x.size() < 3)
                throw std::invalid_argument("Invalid Argument, Need at least three points");

            for (size_t i = 0; i < x.size() - 1; i++)
            {
                if (x[i+1] <= x[i])
                    throw std::invalid_argument("Invalid Argument, X values must be strictly increasing");
            }

            num_points = x.size();
            h_.resize(num_points - 1);
            del_.resize(num_points - 1);
            d_.resize(num_points);


            // Computing the Secant Lines:
            for (int k = 0; k < num_points - 1; k++)
            {
                h_[k] = x_[k + 1] - x_[k];
                del_[k] = (y_[k + 1] - y_[k]) / h_[k];
            }


            auto sgn = [](const double& value)
            {
                return (double(0) < value) - (double(0) > value);
            };

            auto endpoint_correct = [sgn](double d, double del0, double del1)
            {
                if (sgn(d) != sgn(del0))
                {
                    d = 0.0;
                }
                else if (sgn(del0) != sgn(del1) && std::abs(d) > 3.0 * std::abs(del0))
                {
                    d = 3.0 * del0;
                }
                return d;
            };


            // Finding the Derivatives:
            for (int k = 1; k < num_points - 1; k++)
            {
                if (!(sgn(del_[k-1]) == sgn(del_[k])) || (del_[k-1] == 0.0 || del_[k] == 0.0))
                {
                    d_[k] = 0;
                }

                else
                {
                    double w1 = 2 * h_[k] + h_[k - 1];
                    double w2 = h_[k] + 2 * h_[k - 1];

                    d_[k] = (w1 + w2) / (w1/del_[k - 1] + w2/del_[k]);
                }
            }


            d_[0] = ((2 * h_[0] + h_[1]) * del_[0] - h_[0] * del_[1]) / (h_[0] + h_[1]);
            d_[0] = endpoint_correct(d_[0], del_[0], del_[1]);

            d_[num_points - 1] = ((2 * h_[num_points - 2] + h_[num_points - 3]) * del_[num_points - 2]
                                - h_[num_points - 2] * del_[num_points - 3])
                                / (h_[num_points - 2] + h_[num_points - 3]);
            d_[num_points - 1] = endpoint_correct(d_[num_points - 1], del_[num_points - 2], del_[num_points - 3]);
        }

        double operator()(const double& query) const
        {

            auto h1 = [](const double& value)
            {
                return 2.0 * (value * value * value) - 3.0 * (value * value) + 1;
            };

            auto h2 = [](const double& value)
            {
                return -2.0 * (value * value * value) + 3.0 * (value * value);
            };

            auto h3 = [](const double& value)
            {
                return (value * value * value) -2.0 * (value * value) + value;
            };

            auto h4 = [](const double& value)
            {
                return (value * value * value) -1.0 * (value * value);
            };


            if (query > x_[num_points-1] || query < x_[0])
            {
                throw std::invalid_argument("Invalid Argument, Query outside of range");
            }

            else if (query < x_[num_points-1] && query > x_[0])
            {
                auto index = std::upper_bound(x_.begin(), x_.end(), query) - x_.begin() - 1;

                double t = (query - x_[index])/(h_[index]);

                return h1(t) * y_[index] + h2(t) * y_[index + 1] + h3(t) * h_[index] * d_[index] + h4(t) * h_[index] * d_[index + 1];
            }

            else if (query ==  x_[num_points-1])
            {
                return y_[num_points-1];
            }

            else if (query ==  x_[0])
            {
                return y_[0];
            }

            else
            {
                throw std::invalid_argument("Invalid Argument, Query outside of range");
            }
        }

        double return_d_(const int& index){
            return d_[index];
        }
};
