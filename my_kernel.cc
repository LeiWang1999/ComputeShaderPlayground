// GLOBALS: input0:float16[16, 576, 768] -> output0:float16[16, 768]
// BACKEND: c-hlsl_xbox (default)
// CONFIG: {"Foutput0:D0": [-1, 1, 1, 1], "Foutput0:D1": [-1, 1, 1, 1], "Foutput0:O": [0, 1], "Foutput0:S": 0, "Foutput0:R": 0}
// COMPUTE_V1: - einstein_v2("output0[N0, N2] +=! input0[N0, N1, N2]", input_dict={ "input0" : { "dtype" : "float16", "shape" : [16, 576, 768]} })


// ---------------------------------------------------------------------------
// LOCAL: template_op_kernel0 -- input0:float16[16, 576, 768] -> output0:float16[16, 768]

#ifndef __HLSL_COMMON_MACRO__
#define __HLSL_COMMON_MACRO__

#define __ITEM_0_OF__(v) (v).x
#define __ITEM_1_OF__(v) (v).y
#define __ITEM_2_OF__(v) (v).z
#define __ITEM_3_OF__(v) (v).w

#define __STORE_ITEM_0__(t, out, ido, in, idi)  out[(ido) + 0] = in[(idi) + 0]
#define __STORE_ITEM_1__(t, out, ido, in, idi)  out[(ido) + 1] = in[(idi) + 1]
#define __STORE_ITEM_2__(t, out, ido, in, idi)  out[(ido) + 2] = in[(idi) + 2]
#define __STORE_ITEM_3__(t, out, ido, in, idi)  out[(ido) + 3] = in[(idi) + 3]

#define make_int2(x, y) ((int2)(x, y))
#define make_int4(x, y, z, w) ((int4)(x, y, z, w))

#endif


StructuredBuffer<half> input0: register(t0);
RWStructuredBuffer<half> output0: register(u0);

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))")]
[numthreads(1, 1, 1)]
void CSMain(uint3 threadIdx: SV_GroupThreadID, uint3 blockIdx: SV_GroupID, uint3 dispatchIdx: SV_DispatchThreadID) {
  // [thread_extent] blockIdx.x = 16
  // [thread_extent] threadIdx.x = 1
  // [thread_extent] blockIdx.y = 768
  // [thread_extent] threadIdx.y = 1
  //output0[(((((int)blockIdx.x) * 768) + ((int)blockIdx.y)))] = 0.000000e+00f;
  //for (int N1 = 0; N1 < 576; ++N1) {
  //    output0[N1] = N1;
  //  output0[(((((int)blockIdx.x) * 768) + ((int)blockIdx.y)))] = (output0[(((((int)blockIdx.x) * 768) + ((int)blockIdx.y)))] + input0[((((((int)blockIdx.x) * 442368) + (N1 * 768)) + ((int)blockIdx.y)))]);
  //}
  if (blockIdx.x == 0 && blockIdx.y == 0 && blockIdx.z == 0) {
     
     output0[0] = 0.0;
      output0[2] = 2.0;
      output0[1] = 1.0;
      output0[4] = 4.0;
   
  }
}

// Saved Perf = 2.823464e-03 sec / run; Step Produced = 1; Planned Steps = 1;
// Antares Tuning Completed in 1 steps.
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

