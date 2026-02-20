#version 450

layout(location = 0) out vec4 fragColor;

layout(set = 3, binding = 0) uniform CurrentResolution {
	double minX;
	double maxX;
	double minY;
	double maxY;
} res;
const int MAX_ITERATIONS = 2000;

vec3 hsv2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix( vec3(1.0), rgb, c.y);
}

vec3 spectral_color(double l) // RGB <0,1> <- lambda l <400,700> [nm]
{
    double t; 
	double r = 0.0, g = 0.0, b = 0.0;
         if ((l>=400.0)&&(l<410.0)) { t=(l-400.0)/(410.0-400.0); r=    +(0.33*t)-(0.20*t*t); }
    else if ((l>=410.0)&&(l<475.0)) { t=(l-410.0)/(475.0-410.0); r=0.14         -(0.13*t*t); }
    else if ((l>=545.0)&&(l<595.0)) { t=(l-545.0)/(595.0-545.0); r=    +(1.98*t)-(     t*t); }
    else if ((l>=595.0)&&(l<650.0)) { t=(l-595.0)/(650.0-595.0); r=0.98+(0.06*t)-(0.40*t*t); }
    else if ((l>=650.0)&&(l<700.0)) { t=(l-650.0)/(700.0-650.0); r=0.65-(0.84*t)+(0.20*t*t); }
         if ((l>=415.0)&&(l<475.0)) { t=(l-415.0)/(475.0-415.0); g=             +(0.80*t*t); }
    else if ((l>=475.0)&&(l<590.0)) { t=(l-475.0)/(590.0-475.0); g=0.8 +(0.76*t)-(0.80*t*t); }
    else if ((l>=585.0)&&(l<639.0)) { t=(l-585.0)/(639.0-585.0); g=0.84-(0.84*t)           ; }
         if ((l>=400.0)&&(l<475.0)) { t=(l-400.0)/(475.0-400.0); b=    +(2.20*t)-(1.50*t*t); }
    else if ((l>=475.0)&&(l<560.0)) { t=(l-475.0)/(560.0-475.0); b=0.7 -(     t)+(0.30*t*t); }
	return vec3(r, g, b);
}

// Julia
void main()
{
	int i;
	double zr = mix(double(res.minX), double(res.maxX), double(gl_FragCoord.x) / 1000);
	double zi = mix(double(res.minY), double(res.maxY), double(gl_FragCoord.y) / 1000);
	double cr = -0.5125; 
	double ci = 0.5213;
	double temp = 0;
	for (i = 0; i < MAX_ITERATIONS; i++)
	{
		temp = (zr * zr) - (zi * zi) + cr;
		zi = (2 * zr * zi) + ci;
		zr = temp;
		if (zr * zr + zi * zi >= 4.0)
		{
			break;
		}
	}

	// double q = double(i) / double(MAX_ITERATIONS);
	// fragColor = vec4(spectral_color(mix(400.0, 570.0, float(q))), 1);
    float m = float(i) - (log(log(float(sqrt(zr*zr + zi*zi))))) / log(2.0);
	float shade = abs(sin(m / 5.0));
	fragColor = vec4(spectral_color(mix(600, 700, shade)), 1);
} 

// mandelbrot
// void main()
// {
// 	int i;
// 	double cr = mix(double(res.minX), double(res.maxX), double(gl_FragCoord.x) / 1000);
// 	double ci = mix(double(res.minY), double(res.maxY), double(gl_FragCoord.y) / 1000);
// 	double zr = 0; 
// 	double zi = 0;
// 	double temp = 0;
// 	for (i = 0; i < MAX_ITERATIONS; i++)
// 	{
// 		temp = (zr * zr) - (zi * zi) + cr;
// 		zi = (2 * zr * zi) + ci;
// 		zr = temp;
// 		if (zr * zr + zi * zi >= 10000.0)
// 		{
// 			break;
// 		}
// 	}

//     float m = float(i) + 1 - ((log(log(float(sqrt(zr*zr + zi*zi))))) / log(2.0));
// 	float shade = abs(sin(m / 5.0));
// 	fragColor = vec4(spectral_color(mix(500, 700, shade)), 1);
// } 

