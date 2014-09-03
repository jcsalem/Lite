if [ -f ~/.bashrc ]; then
    source ~/.bashrc
fi

pgrep -u root planerun 2>&1 > /dev/null
if [ $? -eq 0 ]; then
  read -p "Kill planerun process? " -n 1 -r
  echo
  if [[ $REPLY =~ ^[Yy]$ ]]; then
    sudo pkill planerun
  fi
fi