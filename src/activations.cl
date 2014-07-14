typedef enum{
    SIGMOID, RELU, LINEAR, RAMP, TANH
}ACTIVATION;

float linear_activate(float x){return x;}
float sigmoid_activate(float x){return 1./(1. + exp(-x));}
float relu_activate(float x){return x*(x>0);}
float ramp_activate(float x){return x*(x>0)+.1*x;}
float tanh_activate(float x){return (exp(2*x)-1)/(exp(2*x)+1);}

float activate(float x, ACTIVATION a, float dropout)
{
    //if((float)rand()/RAND_MAX < dropout) return 0;
    switch(a){
        case LINEAR:
            return linear_activate(x)/(1-dropout);
        case SIGMOID:
            return sigmoid_activate(x)/(1-dropout);
        case RELU:
            return relu_activate(x)/(1-dropout);
        case RAMP:
            return ramp_activate(x)/(1-dropout);
        case TANH:
            return tanh_activate(x)/(1-dropout);
    }
    return 0;
}

__kernel void activate_array(__global float *x,
    const int n, const ACTIVATION a, const float dropout)
{
    int i = get_global_id(0);
    x[i] = activate(x[i], a, dropout);
}
