FROM devkitpro/devkitarm:20241104
# ENV TWLNOPATCHSRLHEADER=1
# Keep this in sync with the "Install tools" and "Setup environment" steps in .github/workflows/nightly.yml
RUN \
  sed -i "/bullseye-security/d" /etc/apt/sources.list && \
  apt-get update && \
  apt-get install -y p7zip-full python && \
  rm -rf /var/lib/apt/lists/* && \
  git config --system --add safe.directory '*'
WORKDIR /data
