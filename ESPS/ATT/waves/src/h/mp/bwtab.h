/* bwtab.h */
/* @(#)bwtab.h	1.3  12/6/89  AT&T, ESI */

double exptab[] = {
1.000000,0.999607,0.999215,0.998823,0.998430,0.998038,0.997647,0.997255,0.996863,0.996472,
0.996081,0.995690,0.995299,0.994908,0.994517,0.994127,0.993737,0.993346,0.992956,0.992566,
0.992177,0.991787,0.991398,0.991009,0.990619,0.990231,0.989842,0.989453,0.989065,0.988676,
0.988288,0.987900,0.987512,0.987125,0.986737,0.986350,0.985962,0.985575,0.985188,0.984801,
0.984415,0.984028,0.983642,0.983256,0.982870,0.982484,0.982098,0.981712,0.981327,0.980942,
0.980557,0.980172,0.979787,0.979402,0.979017,0.978633,0.978249,0.977865,0.977481,0.977097,
0.976713,0.976330,0.975947,0.975563,0.975180,0.974798,0.974415,0.974032,0.973650,0.973268,
0.972885,0.972503,0.972122,0.971740,0.971358,0.970977,0.970596,0.970215,0.969834,0.969453,
0.969072,0.968692,0.968312,0.967931,0.967551,0.967171,0.966792,0.966412,0.966033,0.965653,
0.965274,0.964895,0.964517,0.964138,0.963759,0.963381,0.963003,0.962625,0.962247,0.961869,
0.961491,0.961114,0.960736,0.960359,0.959982,0.959605,0.959228,0.958852,0.958475,0.958099,
0.957723,0.957347,0.956971,0.956595,0.956220,0.955844,0.955469,0.955094,0.954719,0.954344,
0.953969,0.953595,0.953220,0.952846,0.952472,0.952098,0.951724,0.951350,0.950977,0.950604,
0.950230,0.949857,0.949484,0.949111,0.948739,0.948366,0.947994,0.947622,0.947250,0.946878,
0.946506,0.946134,0.945763,0.945392,0.945020,0.944649,0.944279,0.943908,0.943537,0.943167,
0.942796,0.942426,0.942056,0.941686,0.941317,0.940947,0.940578,0.940208,0.939839,0.939470,
0.939101,0.938733,0.938364,0.937996,0.937627,0.937259,0.936891,0.936523,0.936156,0.935788,
0.935421,0.935053,0.934686,0.934319,0.933953,0.933586,0.933219,0.932853,0.932487,0.932121,
0.931755,0.931389,0.931023,0.930658,0.930292,0.929927,0.929562,0.929197,0.928832,0.928467,
0.928103,0.927738,0.927374,0.927010,0.926646,0.926282,0.925919,0.925555,0.925192,0.924828,
0.924465,0.924102,0.923739,0.923377,0.923014,0.922652,0.922290,0.921927,0.921565,0.921204,
0.920842,0.920480,0.920119,0.919758,0.919397,0.919036,0.918675,0.918314,0.917954,0.917593,
0.917233,0.916873,0.916513,0.916153,0.915793,0.915434,0.915074,0.914715,0.914356,0.913997,
0.913638,0.913279,0.912921,0.912562,0.912204,0.911846,0.911488,0.911130,0.910772,0.910415,
0.910057,0.909700,0.909343,0.908986,0.908629,0.908272,0.907915,0.907559,0.907203,0.906846,
0.906490,0.906135,0.905779,0.905423,0.905068,0.904712,0.904357,0.904002,0.903647,0.903292,
0.902938,0.902583,0.902229,0.901874,0.901520,0.901166,0.900813,0.900459,0.900105,0.899752,
0.899399,0.899046,0.898693,0.898340,0.897987,0.897635,0.897282,0.896930,0.896578,0.896226,
0.895874,0.895522,0.895170,0.894819,0.894468,0.894116,0.893765,0.893414,0.893064,0.892713,
0.892363,0.892012,0.891662,0.891312,0.890962,0.890612,0.890262,0.889913,0.889564,0.889214,
0.888865,0.888516,0.888167,0.887819,0.887470,0.887122,0.886773,0.886425,0.886077,0.885729,
0.885381,0.885034,0.884686,0.884339,0.883992,0.883645,0.883298,0.882951,0.882604,0.882258,
0.881911,0.881565,0.881219,0.880873,0.880527,0.880181,0.879836,0.879490,0.879145,0.878800,
0.878455,0.878110,0.877765,0.877421,0.877076,0.876732,0.876388,0.876043,0.875699,0.875356,
0.875012,0.874668,0.874325,0.873982,0.873639,0.873296,0.872953,0.872610,0.872267,0.871925,
0.871583,0.871240,0.870898,0.870556,0.870215,0.869873,0.869531,0.869190,0.868849,0.868508,
0.868167,0.867826,0.867485,0.867144,0.866804,0.866464,0.866123,0.865783,0.865443,0.865104,
0.864764,0.864424,0.864085,0.863746,0.863407,0.863068,0.862729,0.862390,0.862051,0.861713,
0.861375,0.861036,0.860698,0.860361,0.860023,0.859685,0.859348,0.859010,0.858673,0.858336,
0.857999,0.857662,0.857325,0.856988,0.856652,0.856316,0.855980,0.855643,0.855307,0.854972,
0.854636,0.854300,0.853965,0.853630,0.853295,0.852960,0.852625,0.852290,0.851955,0.851621,
0.851286,0.850952,0.850618,0.850284,0.849950,0.849617,0.849283,0.848949,0.848616,0.848283,
0.847950,0.847617,0.847284,0.846952,0.846619,0.846287,0.845954,0.845622,0.845290,0.844958,
0.844627,0.844295,0.843964,0.843632,0.843301,0.842970,0.842639,0.842308,0.841977,0.841647,
0.841316,0.840986,0.840656,0.840326,0.839996,0.839666,0.839336,0.839007,0.838677,0.838348,
0.838019,0.837690,0.837361,0.837032,0.836704,0.836375,0.836047,0.835718,0.835390,0.835062,
0.834734,0.834407,0.834079,0.833752,0.833424,0.833097,0.832770,0.832443,0.832116,0.831789,
0.831463,0.831136,0.830810,0.830484,0.830158,0.829832,0.829506,0.829180,0.828855,0.828529,
0.828204,0.827879,0.827554,0.827229,0.826904,0.826580,0.826255,0.825931,0.825606,0.825282,
0.824958,0.824634,0.824311,0.823987,0.823663,0.823340,0.823017,0.822694,0.822371,0.822048,
0.821725,0.821402,0.821080,0.820757,0.820435,0.820113,0.819791,0.819469,0.819147,0.818826,
0.818504,0.818183,0.817862,0.817541,0.817220,0.816899,0.816578,0.816257,0.815937,0.815617,
0.815296,0.814976,0.814656,0.814336,0.814017,0.813697,0.813378,0.813058,0.812739,0.812420,
0.812101,0.811782,0.811463,0.811145,0.810826,0.810508,0.810190,0.809872,0.809554,0.809236,
0.808918,0.808601,0.808283,0.807966,0.807648,0.807331,0.807014,0.806698,0.806381,0.806064,
0.805748,0.805431,0.805115,0.804799,0.804483,0.804167,0.803851,0.803536,0.803220,0.802905,
0.802590,0.802275,0.801960,0.801645,0.801330,0.801015,0.800701,0.800387,0.800072,0.799758,
0.799444,0.799130,0.798817,0.798503,0.798189,0.797876,0.797563,0.797250,0.796937,0.796624,
0.796311,0.795998,0.795686,0.795373,0.795061,0.794749,0.794437,0.794125,0.793813,0.793502,
0.793190,0.792879,0.792567,0.792256,0.791945,0.791634,0.791323,0.791013,0.790702,0.790392,
0.790081,0.789771,0.789461,0.789151,0.788841,0.788531,0.788222,0.787912,0.787603,0.787294,
0.786985,0.786676,0.786367,0.786058,0.785749,0.785441,0.785133,0.784824,0.784516,0.784208,
0.783900,0.783593,0.783285,0.782977,0.782670,0.782363,0.782055,0.781748,0.781441,0.781135,
0.780828,0.780521,0.780215,0.779909,0.779602,0.779296,0.778990,0.778684,0.778379,0.778073,
0.777768,0.777462,0.777157,0.776852,0.776547,0.776242,0.775937,0.775633,0.775328,0.775024,
0.774719,0.774415,0.774111,0.773807,0.773503,0.773200,0.772896,0.772593,0.772289,0.771986,
0.771683,0.771380,0.771077,0.770774,0.770472,0.770169,0.769867,0.769565,0.769262,0.768960,
0.768659,0.768357,0.768055,0.767754,0.767452,0.767151,0.766850,0.766549,0.766248,0.765947,
0.765646,0.765345,0.765045,0.764744,0.764444,0.764144,0.763844,0.763544,0.763244,0.762945,
0.762645,0.762346,0.762046,0.761747,0.761448,0.761149,0.760850,0.760552,0.760253,0.759955,
0.759656,0.759358,0.759060,0.758762,0.758464,0.758166,0.757868,0.757571,0.757273,0.756976,
0.756679,0.756382,0.756085,0.755788,0.755491,0.755195,0.754898,0.754602,0.754305,0.754009,
0.753713,0.753417,0.753121,0.752826,0.752530,0.752235,0.751939,0.751644,0.751349,0.751054,
0.750759,0.750464,0.750170,0.749875,0.749581,0.749286,0.748992,0.748698,0.748404,0.748110,
0.747817,0.747523,0.747230,0.746936,0.746643,0.746350,0.746057,0.745764,0.745471,0.745178,
0.744886,0.744593,0.744301,0.744009,0.743717,0.743425,0.743133,0.742841,0.742549,0.742258,
0.741966,0.741675,0.741384,0.741093,0.740802,0.740511,0.740220,0.739930,0.739639,0.739349,
0.739058,0.738768,0.738478,0.738188,0.737898,0.737609,0.737319,0.737030,0.736740,0.736451,
0.736162,0.735873,0.735584,0.735295,0.735006,0.734718,0.734429,0.734141,0.733853,0.733565,
0.733277,0.732989,0.732701,0.732413,0.732126,0.731838,0.731551,0.731264,0.730977,0.730690,
0.730403,0.730116,0.729829,0.729543,0.729256,0.728970,0.728684,0.728398,0.728112,0.727826,
0.727540,0.727254,0.726969,0.726683,0.726398,0.726113,0.725828,0.725543,0.725258,0.724973,
0.724689,0.724404,0.724120,0.723835,0.723551,0.723267,0.722983,0.722699,0.722415,0.722132,
0.721848,0.721565,0.721282,0.720998,0.720715,0.720432,0.720149,0.719867,0.719584,0.719302,
0.719019,0.718737,0.718455,0.718173,0.717891,0.717609,0.717327,0.717045,0.716764,0.716482,
0.716201,0.715920,0.715639,0.715358,0.715077,0.714796,0.714516,0.714235,0.713955,0.713674,
0.713394,0.713114,0.712834,0.712554,0.712274,0.711995,0.711715,0.711436,0.711156,0.710877,
0.710598,0.710319,0.710040,0.709761,0.709483,0.709204,0.708926,0.708647,0.708369,0.708091,
0.707813,0.707535,0.707257,0.706980,0.706702,0.706425,0.706147,0.705870,0.705593,0.705316,
0.705039,0.704762,0.704485,0.704209,0.703932,0.703656,0.703380,0.703104,0.702828,0.702552,
0.702276,0.702000,0.701724,0.701449,0.701173,0.700898,0.700623,0.700348,0.700073,0.699798,
0.699523,0.699249,0.698974,0.698700,0.698425,0.698151,0.697877,0.697603,0.697329,0.697055,
0.696782,0.696508,0.696235,0.695961,0.695688,0.695415,0.695142,0.694869,0.694596,0.694323,
0.694051,0.693778,0.693506,0.693234,0.692961,0.692689,0.692417,0.692146,0.691874,0.691602,
0.691331,0.691059,0.690788,0.690517,0.690246,0.689974,0.689704,0.689433,0.689162,0.688892,
0.688621,0.688351,0.688080,0.687810,0.687540,0.687270,0.687000,0.686731,0.686461,0.686192,
0.685922,0.685653,0.685384,0.685115,0.684846,0.684577,0.684308,0.684039,0.683771,0.683502,
0.683234,0.682966,0.682697,0.682429,0.682161,0.681894,0.681626,0.681358,0.681091,0.680823,
0.680556,0.680289,0.680022,0.679755,0.679488,0.679221,0.678954,0.678688,0.678421,0.678155,
0.677889,0.677623,0.677357,0.677091,0.676825,0.676559,0.676293,0.676028,0.675762,0.675497,
0.675232};
