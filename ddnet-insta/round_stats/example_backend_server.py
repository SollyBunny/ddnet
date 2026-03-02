#!/bin/env python3

from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/stats', methods=['POST'])
def stats():
    content_type = request.headers.get('Content-Type', '')
    data = request.get_data(as_text=True) if 'text' in content_type else request.json
    print(f"Received stats: {data}")
    return jsonify({"status": "success", "received": len(data)}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
