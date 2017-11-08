bntseq.o: bntseq.c bntseq.h main.h utils.h kseq.h
bwtsw2_chain.o: bwtsw2_chain.c bwtsw2.h bntseq.h bwt_lite.h bwt.h ksort.h
bwtgap.o: bwtgap.c bwtgap.h bwt.h bntseq.h bwtaln.h bamlite.h stdaln.h
bwtio.o: bwtio.c bwt.h bntseq.h utils.h
bwtmisc.o: bwtmisc.c bntseq.h utils.h main.h bwt.h
bam2bam.o: bam2bam.c bamlite.h bwtaln.h bwt.h bntseq.h stdaln.h bwase.h \
 bwape.h kvec.h khash.h ksort.h main.h bgzf.h utils.h
bamlite.o: bamlite.c bamlite.h
bwt_lite.o: bwt_lite.c bwt_lite.h
is.o: is.c
bwtsw2_core.o: bwtsw2_core.c bwt_lite.h bwtsw2.h bntseq.h bwt.h kvec.h \
 khash.h ksort.h
bgzf.o: bgzf.c bgzf.h khash.h
main.o: main.c main.h utils.h
kstring.o: kstring.c kstring.h
bwase.o: bwase.c stdaln.h bwase.h bntseq.h bwt.h bwtaln.h bamlite.h \
 utils.h kstring.h
bwtindex.o: bwtindex.c bntseq.h bwt.h main.h utils.h
bwape.o: bwape.c bwape.h bwtaln.h bwt.h bntseq.h bamlite.h stdaln.h \
 kvec.h khash.h ksort.h utils.h
bwtaln.o: bwtaln.c bwtaln.h bwt.h bntseq.h bamlite.h stdaln.h bwtgap.h \
 utils.h
bwtsw2_aux.o: bwtsw2_aux.c bntseq.h bwt_lite.h utils.h bwtsw2.h bwt.h \
 stdaln.h kstring.h kseq.h ksort.h
bwtsw2_main.o: bwtsw2_main.c bwt.h bntseq.h bwtsw2.h bwt_lite.h utils.h
cs2nt.o: cs2nt.c bwtaln.h bwt.h bntseq.h bamlite.h stdaln.h
simple_dp.o: simple_dp.c stdaln.h utils.h kseq.h
insert_size.o: insert_size.c bwape.h bwtaln.h bwt.h bntseq.h bamlite.h \
 stdaln.h kvec.h khash.h ksort.h utils.h
stdaln.o: stdaln.c stdaln.h
bwaseqio.o: bwaseqio.c bwtaln.h bwt.h bntseq.h bamlite.h stdaln.h utils.h \
 kseq.h
bwt.o: bwt.c utils.h bwt.h bntseq.h
utils.o: utils.c utils.h
