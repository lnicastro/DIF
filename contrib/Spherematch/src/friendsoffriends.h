long 
friendsoffriends(double x[], double y[], double z[], long nPoints,
								 double linkSep, long **nChunk, long ***chunkList,
								 long *nRa, long nDec, long *firstGroup,
								 long *multGroup, long *nextGroup,
								 long *inGroup, long *nGroups);
long 
chunkfriendsoffriends(double x[], double y[], double z[], long chunkList[],
											long nTargets, double linkSep, long firstGroup[],
											long multGroup[], long nextGroup[],
											long inGroup[], long *nGroups);
