cd ..
git add PER_data.csv
git add PER_full_data.csv
git add PER_data.json
git add PER_full_data.json
git commit -m "updated Peru data $1"
git push
sleep 3
git add ../res/graphs/.
git add ../res/tweets.dat
git commit -m "updated Peru graphs and tweet contents $1"
git push