#!/bin/bash
for  file in ./IEEE-6*.txt; do
	qsub -v filename=${file} param.batch
done
