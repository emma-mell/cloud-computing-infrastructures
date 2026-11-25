FROM python:3.10-bookworm

WORKDIR /app
RUN pip install delegator.py
RUN pip install web.py
RUN apt-get update && apt-get install -y imagemagick
# ImageMagick forbids writing PDF documents by default: allow it.
RUN sed -i 's|<policy domain="coder" rights="none" pattern="PDF" />|<policy domain="coder" rights="read\|write" pattern="PDF" />|' /etc/ImageMagick-6/policy.xml
ADD pdfmagic.py /app
EXPOSE 8080
# Python ignores SIGTERM when it runs as process 1: stop the server with SIGINT, at once.
STOPSIGNAL SIGINT

CMD ["python", "/app/pdfmagic.py"]