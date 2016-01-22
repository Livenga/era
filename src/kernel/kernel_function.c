double
calc_sigmoid(double alpha,
             double input_x) {
  return 1.0 / (1.0 + exp(-alpha * input_x));
}
