#ifndef _NORM_H
#define _NORM_H

#include <iostream>
#include <math.h>

double array[]= {0.000000287, 0.000000302, 0.000000318, 0.000000335, 0.000000352, 0.000000371, 0.000000391, 0.000000411, 0.000000433, 0.000000455, 0.000000479, 0.000000504, 0.000000530, 0.000000558, 0.000000587, 0.000000617, 0.000000649, 0.000000683, 0.000000718, 0.000000755, 0.000000793, 0.000000834, 0.000000876, 0.000000921, 0.000000968, 0.000001017, 0.000001069, 0.000001123, 0.000001179, 0.000001239, 0.000001301, 0.000001366, 0.000001434, 0.000001506, 0.000001581, 0.000001660, 0.000001742, 0.000001828, 0.000001919, 0.000002013, 0.000002112, 0.000002216, 0.000002325, 0.000002439, 0.000002558, 0.000002682, 0.000002813, 0.000002949, 0.000003092, 0.000003241, 0.000003398, 0.000003561, 0.000003732, 0.000003911, 0.000004098, 0.000004294, 0.000004498, 0.000004712, 0.000004935, 0.000005169, 0.000005413, 0.000005668, 0.000005934, 0.000006212, 0.000006503, 0.000006807, 0.000007124, 0.000007455, 0.000007801, 0.000008163, 0.000008540, 0.000008934, 0.000009345, 0.000009774, 0.000010221, 0.000010689, 0.000011176, 0.000011685, 0.000012215, 0.000012769, 0.000013346, 0.000013948, 0.000014575, 0.000015230, 0.000015912, 0.000016624, 0.000017365, 0.000018138, 0.000018944, 0.000019783, 0.000020658, 0.000021569, 0.000022518, 0.000023507, 0.000024536, 0.000025609, 0.000026726, 0.000027888, 0.000029099, 0.000030359, 0.000031671, 0.000033037, 0.000034458, 0.000035936, 0.000037475, 0.000039076, 0.000040741, 0.000042473, 0.000044274, 0.000046148, 0.000048096, 0.000050122, 0.000052228, 0.000054418, 0.000056694, 0.000059059, 0.000061517, 0.000064072, 0.000066726, 0.000069483, 0.000072348, 0.000075324, 0.000078414, 0.000081624, 0.000084957, 0.000088417, 0.000092010, 0.000095740, 0.000099611, 0.000103630, 0.000107800, 0.000112127, 0.000116617, 0.000121275, 0.000126108, 0.000131120, 0.000136319, 0.000141711, 0.000147302, 0.000153099, 0.000159109, 0.000165339, 0.000171797, 0.000178491, 0.000185427, 0.000192616, 0.000200064, 0.000207780, 0.000215773, 0.000224053, 0.000232629, 0.000241510, 0.000250707, 0.000260229, 0.000270088, 0.000280293, 0.000290857, 0.000301791, 0.000313106, 0.000324814, 0.000336929, 0.000349463, 0.000362429, 0.000375841, 0.000389712, 0.000404058, 0.000418892, 0.000434230, 0.000450087, 0.000466480, 0.000483424, 0.000500937, 0.000519035, 0.000537737, 0.000557061, 0.000577025, 0.000597648, 0.000618951, 0.000640953, 0.000663675, 0.000687138, 0.000711364, 0.000736375, 0.000762195, 0.000788846, 0.000816352, 0.000844739, 0.000874032, 0.000904255, 0.000935437, 0.000967603, 0.001000782, 0.001035003, 0.001070294, 0.001106685, 0.001144207, 0.001182891, 0.001222769, 0.001263873, 0.001306238, 0.001349898, 0.001394887, 0.001441242, 0.001488999, 0.001538195, 0.001588870, 0.001641061, 0.001694810, 0.001750157, 0.001807144, 0.001865813, 0.001926209, 0.001988376, 0.002052359, 0.002118205, 0.002185961, 0.002255677, 0.002327400, 0.002401182, 0.002477075, 0.002555130, 0.002635402, 0.002717945, 0.002802815, 0.002890068, 0.002979763, 0.003071959, 0.003166716, 0.003264096, 0.003364160, 0.003466974, 0.003572601, 0.003681108, 0.003792562, 0.003907033, 0.004024589, 0.004145301, 0.004269243, 0.004396488, 0.004527111, 0.004661188, 0.004798797, 0.004940016, 0.005084926, 0.005233608, 0.005386146, 0.005542623, 0.005703126, 0.005867742, 0.006036558, 0.006209665, 0.006387155, 0.006569119, 0.006755653, 0.006946851, 0.007142811, 0.007343631, 0.007549411, 0.007760254, 0.007976260, 0.008197536, 0.008424186, 0.008656319, 0.008894043, 0.009137468, 0.009386706, 0.009641870, 0.009903076, 0.010170439, 0.010444077, 0.010724110, 0.011010658, 0.011303844, 0.011603792, 0.011910625, 0.012224473, 0.012545461, 0.012873721, 0.013209384, 0.013552581, 0.013903448, 0.014262118, 0.014628731, 0.015003423, 0.015386335, 0.015777607, 0.016177383, 0.016585807, 0.017003023, 0.017429178, 0.017864421, 0.018308900, 0.018762766, 0.019226172, 0.019699270, 0.020182215, 0.020675163, 0.021178270, 0.021691694, 0.022215594, 0.022750132, 0.023295468, 0.023851764, 0.024419185, 0.024997895, 0.025588060, 0.026189845, 0.026803419, 0.027428950, 0.028066607, 0.028716560, 0.029378980, 0.030054039, 0.030741909, 0.031442763, 0.032156775, 0.032884119, 0.033624969, 0.034379502, 0.035147894, 0.035930319, 0.036726956, 0.037537980, 0.038363570, 0.039203903, 0.040059157, 0.040929509, 0.041815138, 0.042716221, 0.043632937, 0.044565463, 0.045513977, 0.046478658, 0.047459682, 0.048457226, 0.049471468, 0.050502583, 0.051550748, 0.052616138, 0.053698928, 0.054799292, 0.055917403, 0.057053433, 0.058207556, 0.059379941, 0.060570758, 0.061780177, 0.063008364, 0.064255488, 0.065521712, 0.066807201, 0.068112118, 0.069436623, 0.070780877, 0.072145037, 0.073529260, 0.074933700, 0.076358510, 0.077803841, 0.079269841, 0.080756659, 0.082264439, 0.083793322, 0.085343451, 0.086914962, 0.088507991, 0.090122672, 0.091759136, 0.093417509, 0.095097918, 0.096800485, 0.098525329, 0.100272568, 0.102042315, 0.103834681, 0.105649774, 0.107487697, 0.109348552, 0.111232437, 0.113139446, 0.115069670, 0.117023196, 0.119000107, 0.121000484, 0.123024403, 0.125071936, 0.127143151, 0.129238112, 0.131356881, 0.133499513, 0.135666061, 0.137856572, 0.140071090, 0.142309654, 0.144572300, 0.146859056, 0.149169950, 0.151505003, 0.153864230, 0.156247645, 0.158655254, 0.161087060, 0.163543059, 0.166023246, 0.168527607, 0.171056126, 0.173608780, 0.176185542, 0.178786380, 0.181411255, 0.184060125, 0.186732943, 0.189429655, 0.192150202, 0.194894521, 0.197662543, 0.200454193, 0.203269392, 0.206108054, 0.208970088, 0.211855399, 0.214763884, 0.217695438, 0.220649946, 0.223627292, 0.226627352, 0.229649997, 0.232695092, 0.235762498, 0.238852068, 0.241963652, 0.245097094, 0.248252230, 0.251428895, 0.254626915, 0.257846111, 0.261086300, 0.264347292, 0.267628893, 0.270930904, 0.274253118, 0.277595325, 0.280957309, 0.284338849, 0.287739719, 0.291159687, 0.294598516, 0.298055965, 0.301531788, 0.305025731, 0.308537539, 0.312066949, 0.315613697, 0.319177509, 0.322758110, 0.326355220, 0.329968554, 0.333597821, 0.337242727, 0.340902974, 0.344578258, 0.348268273, 0.351972708, 0.355691245, 0.359423567, 0.363169349, 0.366928264, 0.370699981, 0.374484165, 0.378280478, 0.382088578, 0.385908119, 0.389738752, 0.393580127, 0.397431887, 0.401293674, 0.405165128, 0.409045885, 0.412935577, 0.416833837, 0.420740291, 0.424654565, 0.428576284, 0.432505068, 0.436440537, 0.440382308, 0.444329995, 0.448283213, 0.452241574, 0.456204687, 0.460172163, 0.464143607, 0.468118628, 0.472096830, 0.476077817, 0.480061194, 0.484046563, 0.488033527, 0.492021686, 0.496010644, 0.5};

double ncdf(double x){
  if(x==0.0){
    return 0.5;
  }
  else if (x>0){
    return (1-ncdf(-x));
  }
  else if ((x>=-5.0)&&(x<0)){
    int ctr = 0;
    double step=0.01;
    double ret_value;
    for (double i = -5.0; i<0.0; i+=step, ctr++){
      if((x>=i)&&(x<i+step)){
	  ret_value = array[ctr]+(array[ctr+1]-array[ctr])*(x-i)/step;
	  break;
      }
    }
    return ret_value;
  }
  else if(x<-5.0){
    return array[0];
  }
}

double npdf(double x){
  const double pi_const = 0.398942280;
  return pi_const * exp(-0.5*x*x);
}

double getinversenorm(double x){
  
  if(x>0.5)
    return -getinversenorm(1.0-x);

  double return_value=0.0;

  switch ((int)round(x*100)){
  case 50:
    return_value = 0.0;
    break;
  case 25:
    return_value = -0.6744897502;
    break;
  case 10:
    return_value = -1.2815515655;
    break;
  }  

  return return_value;
}

#endif