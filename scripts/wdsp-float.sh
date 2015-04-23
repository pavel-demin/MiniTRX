for file in wdsp/*.c wdsp/*.h
do
  echo $file
  dos2unix $file
  sed -i 's/double/float/g; s/fftw_/fftwf_/g' $file
done
