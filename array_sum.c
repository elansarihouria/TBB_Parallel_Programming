#include <iostream>
 2 #include <stdlib.h>
 3 #include "tbb/task_scheduler_init.h"
 4 #include "tbb/parallel_for.h"
 5 #include "tbb/blocked_range.h"
 6 #include "fatals.h"
 7 #include "hrtime.h"
 8
 9 using namespace tbb;
10 using namespace std;
11
12 /*------------------------------------------------------+
13  | This may look like a class definition, but its not:  |
14  | This is actually a TBB Parallel For BODY definition! |
15  |                                                      |
16  | Arguments are the data members of the "class", in    |
17  | this case, p_array_a, p_array_b, and p_array_sum,    |
18  | which are all of type int *                          |
19  *------------------------------------------------------*/
20 class ArraySummer {
21
22   int * p_array_a;
23   int * p_array_b;
24   int * p_array_sum;
25
26 public:
27   // This empty constructor with an initialization list is used to setup calls to the function
28   ArraySummer(int * p_a, int * p_b, int * p_sum) : p_array_a(p_a), p_array_b(p_b), p_array_sum(p_sum) { }
29
30   /*----------------------------------------------------------+
31    | Here is the actual body, that will be called in parallel |
32    | by the TBB runtime. You MUST put this code inside the    |
33    | class definition, since the compiler will be expanding   |
34    | and inlining this code as part of the template process.  |
35    |                                                          |
36    | The blocked_range<int> is something like a list of       |
37    | indexes corresponding to each invocation of the function |
38    +----------------------------------------------------------*/
39
40   void operator() ( const blocked_range<int>& r ) const {
41     for ( int i = r.begin(); i != r.end(); i++ ) { // iterates over the entire chunk
42       p_array_sum[i] = p_array_a[i] + p_array_b[i];
43     }
44   }
45
46 };
47
48 /*------------------------------------------------+
49  | Here's the meat of the program -- sums two     |
50  | Fibonacci-like arrays, using a for loop first  |
51  | and then a parallel_for TBB template.          |
52  +------------------------------------------------*/
53
54 int main(size_t argc, char *argv[]) {
55  hrtime starttime, endtime, singlethread_time, tbb_time; // for timing
56  int * p_A;
57  int * p_B;
58  int * p_SUM_1T;
59  int * p_SUM_TBB;
60
61  /* This is the TBB runtime... */
62  task_scheduler_init init;
63
64  if( argc != 2 ) {
65    fatal("Usage: %s <arraySize>\n",argv[0]);
66  }
67
68  int nElements = atoi( argv[1] );
69  if( nElements <= 2 ) {
70    fatal("Array size (%s) must be an integer > 2\n", argv[1]);
71  }
72
73  p_A       = new int[nElements];
74  p_B       = new int[nElements];
75  p_SUM_1T  = new int[nElements];
76  p_SUM_TBB = new int[nElements];
77
78  /*
79   * Initialize the data sets ... could do this in parallel too, but
80   * serial is easier to read
81   */
82  p_A[0] = p_B[0] = 0;
83  p_A[1] = p_B[1] = 1;
84  for( int i=2;i<nElements;i++) {
85    p_A[i]   = (p_A[i-1] + p_A[i-2]) % (INT_MAX/2);
86    p_B[i]   = p_A[i];
87    p_SUM_1T[i] = 0;
88    p_SUM_TBB[i] = 0;
89  }
90
91
92  /*
93   * Time how long it takes to sum the arrays using a single thread
94   */
95  starttime = gethrtime();
96
97  for( int i=0;i<nElements;i++ ) {
98    p_SUM_1T[i] = p_A[i] + p_B[i];
99  }
100
101  endtime = gethrtime();
102  singlethread_time = endtime - starttime;
103
104  /*
105   * Now sum the arrays again using TBB, again timing the execution
106   */
107  starttime = gethrtime();
108
109  parallel_for(blocked_range<int>(0, nElements, 100),
110        ArraySummer( p_A, p_B, p_SUM_TBB ) );
111
112  endtime = gethrtime();
113  tbb_time = endtime - starttime;
114
115  /*
116   * Verify the sums match
117   */
118  for(int i=0;i<nElements;i++) {
119    if( p_SUM_1T[i] != p_SUM_TBB[i] ) {
120      cout << p_A[i] << " + " << p_B[i] << " = " << p_SUM_1T[i] << " AND " << p_SUM_TBB[i] <<  endl;
121    }
122  }
123
124  /*
125   * Print the times
126   */
127  cout << "1T summing time:  " << singlethread_time << " ticks" << endl;
128  cout << "TBB summing time: " << tbb_time << " ticks" << endl;
129
130  delete [] p_A;
131  delete [] p_B;
132  delete [] p_SUM_1T;
133  delete [] p_SUM_TBB;
134
135  return 0;
136 }
137
